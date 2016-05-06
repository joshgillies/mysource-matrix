<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*$Id: deja_vu.inc,v 1.12 2013/08/20 00:45:31 cupreti Exp $
*
*/

// We use a control file to enable/disable Deja Vu, since it must be flushed
// each time it is turned off. See scripts/dejavu_management.php
define('SQ_DEJA_VU_CONTROL', SQ_DATA_PATH.'/private/conf/.dejavu');
define('SQ_DEJA_VU_CACHE_PREFIX', 'sq_deja_vu|');

// Storage types
define('SQ_DEJA_VU_ASSET', 'asset');
define('SQ_DEJA_VU_LOCALE', 'locale_manager');

require_once SQ_LIB_PATH.'/MatrixRedis/MatrixRedis.inc';

/**
* Deja_Vu
*
* The blurry lines between short term and long term memory. Ever thought you'd seen that 
* black cat before?
*
* Deja_Vu provides a lazy memory store via memcached for asset objects and metadata values. 
* It is designed to move common and predictable data loading into a more persistent
* memory, purely to improve scalability.
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.12 $
* @package MySource_Matrix
*/
class Deja_Vu extends MySource_Object
{
    /**
     * Memcache
     *
     * @var Object Memcache
     */
    private $_memcache;

    /**
     * Redis handler.
     */
    private $_redis = NULL;


    /**
     * Enabled flag.
     *
     * @var int
     */
    private $_enabled;


    /**
     * Time period in seconds to wait before reaching
     *
     * @var int
     */
    private $_recaching_delay;

    /**
     * Which cache storage handler is being used.
     */
    private $_cache_storage = 'memcache';


    /**
    * Constructor
    *
    * Runs a bunch of assertions for the config, which will throw fatals. 
    * 
    * @return Deja_Vu
    * @access public
    */
    function __construct()
    {
        $this->_enabled = 0;
        $this->_recaching_delay = 0;
        if (is_file(SQ_DEJA_VU_CONTROL)) {
            $config = explode(',', file_get_contents(SQ_DEJA_VU_CONTROL));
            $this->_enabled = (int) array_get_index($config, 0, 0);
            $this->_recaching_delay = array_get_index($config, 1, 0);
            $this->_cache_storage = trim(array_get_index($config, 2, 'memcache'));

            if ($this->_enabled !== 1) $this->_enabled = 0;
        }

        if ($this->enabled() === 1) {
            $this->checkRequirements();
            // Register for updates, so that we can keep our memories fresh.
            $em = $GLOBALS['SQ_SYSTEM']->getEventManager();
            $em->addEventListener($this, Array(    'AssetStatusUpdate',
                                                'assetDeleted',
                                                'AssetUpdate',
                                                'ContentsUpdated',
                                                'attributeChange',
                                                'AssetTypeUpdate',
                                                'MetadataUpdate',
                                                'MetadataDeleted',
                                                )
                                            );
        }

    }//end constructor


    /**
     * Check the requirements (depending on the cache type), and set up the local connection handler.
     *
     * @return void
     */
    public function checkRequirements()
    {
        if ($this->_cache_storage === 'memcache') {
            assert_true(extension_loaded('memcache'), 'Cannot use Deja Vu; it requires the memcache PECL extension installed within , which is not installed');
            assert_true(file_exists(SQ_DATA_PATH.'/private/conf/memcache.inc'), 'Cannot use Deja Vu; the Memcache configuration file is not set');

            $memcache_conf = require(SQ_DATA_PATH.'/private/conf/memcache.inc');
            $hosts =& $memcache_conf['hosts'];
            $services =& $memcache_conf['services'];

            assert_true(count($hosts) > 0, 'Cannot use Deja Vu; no hosts are defined in the Memcache configuration file');
            assert_true(array_key_exists('deja_vu', $services) === TRUE, 'Cannot use Deja Vu; no Memcache hosts are assigned');
            assert_true(count($services['deja_vu']) > 0, 'Cannot use Deja Vu; no Memcache hosts are assigned');

            // If has the memcache module installed, instantiate it and try to load some config.
            $this->_memcache = new Memcache;

            foreach ($services['deja_vu'] as $host_key => $weight) {
                assert_true(array_key_exists($host_key, $hosts) === TRUE, 'Cannot use Deja Vu; host key "'.$host_key.'" assigned for use for Deja Vu but not defined as a host');
                $host = $hosts[$host_key];
                $this->_memcache->addServer($host['host'], $host['port'], $host['persistent'], $weight, $host['timeout'], $host['retry_interval'], $host['status'],  Array($this, 'failureCallback'));
            }

            $this->_memcache->setCompressThreshold($memcache_conf['compression_threshold'], $memcache_conf['compression_min_saving']);
        } else {
            $this->_redis = MatrixRedis::getConnection('deja_vu');
        }

    }//end checkRequirements()


    /**
     * Flag to control enabled/disabled state.
     *
     * @return bool
     * @access public
     */
    public function enabled()
    {
        return $this->_enabled;

    }//end enabled()


    /**
     * Get the recaching delay time value
     *
     * @return bool
     * @access public
     */
    public function getRecachingDelay()
    {
        return $this->_recaching_delay;

    }//end getRecachingDelay()


    /**
     * Get the cache storage type.
     *
     * @return string
     * @access public
     */
    public function getCacheStorage()
    {
        return $this->_cache_storage;

    }//end getCacheType()


    /**
    * Fetch an item from memcached, or null if not found.
    *
    * @param String $type The type of data sort, asset or metadata currently.
    * @param String $assetid 
    *
    * @return mixed (Object or Array)
    * @access public
    */
    public function recall($type, $assetid = NULL, $contextid = NULL)
    {
        //ignore shadow assets
        if (strpos($assetid, ':') !== FALSE) return NULL;
        // bug fix #5831 Inconsistent contents when using Deja Vu and Rollback
        // If in rollback don't worry about getting cached content for assets
        if (defined('SQ_ROLLBACK_VIEW') && SQ_ROLLBACK_VIEW) return null;
        $cache_key = $this->_getCacheKey($type, $assetid, $contextid);
        switch ($type) {
        case 'asset':
            $obj = $this->_get($cache_key);
            if (!$obj || is_string($obj)) {
                $asset_cache_key = $this->_getAssetCacheKey($assetid);
                $cache_update_info = explode(',', $this->_get($asset_cache_key));
                $update_ts = array_get_index($cache_update_info, 0, '0');
                // Cache miss means asset will be recached. Set the recaching flag
                $this->_set($asset_cache_key, $update_ts.',1');

                // Cache miss for $cache_key
                return NULL;
            }

            // If $obj is an undefined object, it's an asset stored without the class being included yet.
            // We must include the correct asset and try again.
            if ($obj instanceof __PHP_Incomplete_Class) {
                $obj_def = (array) $obj;
                // Determine the correct type code
                $type_code = strtolower($obj_def['__PHP_Incomplete_Class_Name']);

                $am    =& $GLOBALS['SQ_SYSTEM']->am;
                if ($am->installed($type_code)) {
                    $am->includeAsset($type_code);

                    // We need to re-get the object to re-unserialize.
                    $obj = $this->_get($cache_key);
                } else {
                    // Should never happen, but just in case..
                    log_error('Deja Vu could not load asset type "'.$type_code.'" for #'.$assetid.'. Assuming the asset type has been uninstalled, so forgetting this asset.');
                    $this->forget($assetid, $type);
                    return NULL;
                }
            }//end if

            return $obj;
            break;

        default:
            $data = $this->_get($cache_key);

            if (!$data) {
                return NULL;
            } else if ($data instanceof __PHP_Incomplete_Class) {
                return NULL;
            } else {
                return $data;
            }
            break;

        }//end switch

    }//end recall


    /**
    * Store an item in memcached.
    *
    * @param String $type    The type of data sort, asset or metadata currently.
    * @param String $assetid
    * @param Mixed  $object  The object to store. Which might not be an Object.
    *
    * @return boolean
    * @access public
    */
    public function remember($type, $assetid, $object, $contextid = NULL)
    {
        //ignore shadow assets
        if (strpos($assetid, ':') !== FALSE) return NULL;
        // bug fix #5831 Inconsistent contents when using Deja Vu and Rollback
        // If in rollback we shouldn't be really caching the content for asset
        if (defined('SQ_ROLLBACK_VIEW') && SQ_ROLLBACK_VIEW) return null;

        // Do not re-cache if some concurrent process has just updated this asset
        // or if the expected db replication delay has not elapsed yet
        if ($type == 'asset') {
            $cache_update_info = explode(',', $this->_get($this->_getAssetCacheKey($assetid)));
            $updated_time = array_get_index($cache_update_info, 0, '0');
            $recaching = array_get_index($cache_update_info, 1, '1');

            $current_time = microtime(TRUE);
            if (!$recaching || ($this->_recaching_delay && $updated_time && ($current_time - $updated_time) <= $this->_recaching_delay)) {
                return NULL;
            }
        }

        return $this->_set($this->_getCacheKey($type, $assetid, $contextid), $object);

    }//end remember()


    /**
    * Remove an item from memcached
    *
    * @param string $itemid    id of the item to remove
    *
    * @return boolean    false if the item is not in the cache or failure
    * @access public
    */
    public function forget($type, $assetid = NULL, $contextid = NULL)
    {
        $contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
        foreach($contextids as $contextid) {
            $this->_delete($this->_getCacheKey($type, $assetid, $contextid));
        }//end foreach

        // Mark the deletion timestamp and unset the recaching flag
        // Note that we intentionally use same cache key all the contexts
        if ($type == 'asset') {
            $this->_set($this->_getAssetCacheKey($assetid), microtime(TRUE).',0');
        }

        return TRUE;

    }//end forget()


    /**
     * Forget everything.
     *
     * @return bool Success|Failure
     * @access public
     */
    public function forgetAll()
    {
        switch ($this->_cache_storage) {
            case 'memcache':
                return $this->_memcache->flush();
            break;

            case 'redis':
                return $this->_redis->flushDB();
            break;
        }

    }//end forgetAll()


    /**
     * Generate the cache key for the given type / assetid.
     *
     * Assetid may be null since it's not always relevant.
     *
     * @param string $type
     * @param string $assetid
     * @return string
     * @access private
     */
    private function _getCacheKey($type, $assetid = NULL, $contextid = NULL)
    {
        switch ($type) {
            case SQ_DEJA_VU_LOCALE:
                return SQ_DEJA_VU_CACHE_PREFIX.'|'.$type;
                break;
            default:
                $contextid = is_null($contextid) ? $GLOBALS['SQ_SYSTEM']->getContextId() : $contextid;
                return SQ_DEJA_VU_CACHE_PREFIX.$contextid.'\\'.$assetid.'|'.$type;
                break;
        }

    }//end _getCacheKey()


    /**
     * Generate the cache key for the given assetid disregard to the context
     *
     * @param string $assetid
     *
     * @return string
     * @access private
     */
    private function _getAssetCacheKey($assetid)
    {
        if (empty($assetid)) {
            return FALSE;
        }

        return SQ_DEJA_VU_CACHE_PREFIX.$assetid.'|update_info';

    }//end _getAssetCacheKey()


    /******* EVENT HANDLERS *************************************************/


    /**
     * Event handler for when asset status changes.
     *
     * @param Object broadcaster
     * @param Array     vars
     * @return void
     * @access public
     */
    public function onAssetStatusUpdate(&$broadcaster, $vars)
    {
        // Status is non contextable, so forget everything..
        $contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
        foreach ($contexts as $contextid => $context) {
            $this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id, $contextid);
        }

    }//end onAssetStatusUpdate

    /**
     * Event handler for when asset is deleted.
     *
     * @param Object broadcaster
     * @param Array     vars
     * @return void
     * @access public
     */
    public function onAssetDeleted(&$broadcaster, $vars)
    {
        $contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
        foreach ($contexts as $contextid => $context) {
            $this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id, $contextid);
        }

    }//end onAssetDeleted


    /**
     * Event handler for when asset is updated.
     *
     * @param Object broadcaster
     * @param Array     vars
     * @return void
     * @access public
     */
    public function onAssetUpdate(&$broadcaster, $vars = Array())
    {
        $this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);

    }//end onAssetUpdate


    /**
     * Event handler for when asset's contents are updated.
     *
     * @param Object broadcaster
     * @param Array     vars
     * @return void
     * @access public
     */
    public function onContentsUpdated(&$broadcaster, $vars)
    {
        $this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);

    }//end onContentsUpdated


    /**
     * Event handler for when asset attributes change.
     *
     * @param Object broadcaster
     * @param Array     vars
     * @return void
     * @access public
     */
    public function onAttributeChange(&$broadcaster, $vars)
    {
        $this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);

    }//end onAttributeChange


    /**
     * Event handler for when asset type changes (morphing).
     *
     * @param Object broadcaster
     * @param Array     vars
     * @return void
     * @access public
     */
    public function onAssetTypeUpdate(&$broadcaster, $vars = Array())
    {
        $contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
        foreach ($contexts as $contextid => $context) {
            $this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id, $contextid);
        }

    }//end onAssetTypeUpdate


    /******* UTILITIES *************************************************/


    /**
     * Enable DeJa Vu
     *
     * @return bool Success|Failure
     * @access public
     */
    public function enable()
    {
        if (!$this->enabled()) {
            $rv = (bool) file_put_contents(SQ_DEJA_VU_CONTROL, '1'.','.$this->_recaching_delay.','.$this->_cache_storage);
            if ($rv) {
                $this->_enabled = 1;
            }
        }
        return $rv;

    }//end enable()


    /**
     * Disable DeJa Vu
     *
     * @return bool Success|Failure
     * @access public
     */
    public function disable()
    {
        $rv = FALSE;
        if ($this->enabled()) {
            $rv = (bool) file_put_contents(SQ_DEJA_VU_CONTROL, '0'.','.$this->_recaching_delay.','.$this->_cache_storage);
            if ($rv) {
                $this->_enabled = 0;
            }
        }
        return $rv;

    }//end disable()


    /**
     * Set Deja Vu recaching delay time
     *
     * @return bool
     * @access public
     */
    public function setRecachingDelay($period)
    {
        $rv = (bool) file_put_contents(SQ_DEJA_VU_CONTROL, $this->_enabled.','.$period.','.$this->_cache_storage);
        if ($rv) {
            $this->_recaching_delay = $period;
        }

        return $rv;

    }//end setRecachingDelay()


    /**
     * Set Deja Vu cache type.
     *
     * @return bool
     * @access public
     */
    public function setCacheStorage($cacheStorage)
    {
        if (in_array($cacheStorage, array('memcache', 'redis')) === FALSE) {
            log_error(get_class().' invalid cache type ('.$cacheStorage.')', E_USER_ERROR);
            return FALSE;
        }

        $rv = (bool) file_put_contents(SQ_DEJA_VU_CONTROL, $this->_enabled.','.$this->_recaching_delay.','.$cacheStorage);
        if ($rv) {
            $this->_cache_storage = $cacheStorage;
        }

        return $rv;

    }//end setCacheStorage()


    /**
    * Failure Callback function for Memcache::addServer
    *
    * @param $hostname
    * @param $port
    *
    * @return void
    * @access public
    */
    function failureCallback($hostname, $port)
    {
        log_error(get_class()." failure communicating with $hostname:$port", E_USER_WARNING);

    }//end failureCallback()


    /**
     * Get an item from cache storage depending on which storage is being used.
     *
     * @param string $item The item to get.
     *
     * @return mixed
     */
    private function _get($item)
    {
        switch ($this->_cache_storage) {
            case 'memcache':
                return $this->_memcache->get($item);
            break;

            case 'redis':
                // don't return an object if redis is down
                if (!is_object($this->_redis)) return NULL;
                $val = $this->_redis->get($item);
                // This is probably too big, but because of potentially long class names,
                // it's set this long to make the next match a bit more specific.
                $prefix = substr($val, 0, 100);
                if (preg_match('/^(O|a)\:\d+\:/', $prefix)) {
                    $val = unserialize($val);
                }
                return $val;
            break;
        }
    }//end _get()


    /**
     * Set an item from cache storage depending on which storage is being used.
     *
     * @param string $item The item to set.
     * $param mixed  $val  The value to set the item to.
     *
     * @return mixed
     */
    private function _set($item, $val)
    {
        switch ($this->_cache_storage) {
            case 'memcache':
                return $this->_memcache->set($item, $val);
            break;

            case 'redis':
                // don't return an object if redis is down
                if (!is_object($this->_redis)) return;
                if (is_array($val) || is_object($val)) {
                    $val = serialize($val);
                }
                return $this->_redis->set($item, $val);
            break;
        }

    }//end _set()


    /**
     * Delete an item from cache storage depending on which storage is being used.
     *
     * @param string $item The item to delete.
     *
     * @return mixed
     */
    private function _delete($item)
    {
        switch ($this->_cache_storage) {
            case 'memcache':
                return $this->_memcache->delete($item);
            break;

            case 'redis':
                return $this->_redis->delete($item);
            break;
        }

    }//end _delete()


}//end class

?>
