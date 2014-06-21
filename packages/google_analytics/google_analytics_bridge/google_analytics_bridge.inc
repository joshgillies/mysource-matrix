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
* $Id: data_record.inc,v 1.9 2013/01/29 06:42:16 akarelia Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* Google Analytics Bridge
*
* Purpose
*
*
* @author  Squiz Pty Ltd <products@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage google_analytics
*/
class Google_Analytics_Bridge extends OAuth2_Token implements Bridge
{


    /**
     * The authentication endpoint.
     *
     * The "access_type=offline" query param is to ensure we get a refresh
     * token. Google doesn't provide this by default.
     *
     * @const string
     */
    const AUTH_ENDPOINT = 'https://accounts.google.com/o/oauth2/auth?access_type=offline';
    
    /**
     * The token endpoint.
     *
     * @const string
     */
    const TOKEN_ENDPOINT = 'https://accounts.google.com/o/oauth2/token';
    
    /**
     * Space-separated list of scopes desired by this bridge.
     *
     * The plain-English permissions shown to users by Google when
     * requesting permission are, in order:
     * - View your Google Analytics data
     * - Edit Google Analytics management entities
     *
     * User Management and Provisioning API access is not requested.
     *
     * @const string
     */
    const DEFAULT_SCOPE = 'https://www.googleapis.com/auth/analytics.readonly https://www.googleapis.com/auth/analytics.edit';
    
    /**
     * Default fetch prefix for Analytics API data.
     *
     * @const string
     */
    const FETCH_PREFIX = 'https://www.googleapis.com/analytics/v3';
    
    
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// Certain items of the token storage are not meant to be user-updateable.
		$this->setAttrValue('authorization_endpoint', self::AUTH_ENDPOINT);
		$this->setAttrValue('token_endpoint', self::TOKEN_ENDPOINT);
		$this->setAttrValue('scope', self::DEFAULT_SCOPE);
		$this->setAttrValue('token_storage', 'global');

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		
		return TRUE;

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2]['google_analytics_property'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $links;

	}//end _getAllowedLinks()
	
	
//--      FRONTEND PAINTING        --//

    
	/**    
	 * Outputs the requested resource(s).
	 *  
	 * @return void    
	 * @access public    
	 */    
	public function printFrontend()
	{
		// don't let browser/proxy cache this asset
		header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
		header('Pragma: no-cache');
		header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
		
        // start authentication (if required or forced)
        // Unlike the OAuth2 Token, "force=true" is required, not "force" with
        // any value.
        // Eventually force won't exist, revoking of token to occur in the
        // edit interface because we have a fixed place to do this.
        $force = FALSE;
        if (array_get_index($_GET, 'force') === 'true') {
            $force = TRUE;
        }
        
        $token = $this->getAccessToken($force);
        return parent::printFrontend();
        
	}//end printFrontend()
	
	
	/**
	 * Paint the body once authenticated.
	 */
	public function printBody()
	{
	    echo '<pre style="word-wrap: break-word; white-space: pre-wrap; font-family: monospace">';
	    
	    
	    
	    echo '</pre>';
	    
	}//end printBody()
	
	
//--      ANALYTICS REQUESTING        --//


    /**
     *
     */
    public function getProperties()
    {
        // Cache for 4 hours.
        $cm       = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key      = 'accountSummaries';
        $response = Array();
        
        $refresh_resp = $this->loadFromFetchCache($this->getRefreshKey($key));
        if ((string) $refresh_resp === '1') {
            // Not stale yet.
            $response = $this->loadFromFetchCache($key);
        } else {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $url      = self::FETCH_PREFIX.'/management/accountSummaries';
            $response = $this->fetchWithBackoff($url);
            
            if ($response !== NULL) {
                $this->saveToFetchCache($key, $response);
            } else {
                $response = $this->loadFromFetchCache($key);
            }
        }//end if
        
        
        if (($response === FALSE) || ($response === NULL)) {
            $props = Array();
        } else {
            foreach (array_get_index($response, 'items', Array()) as $account) {
                foreach ($account['webProperties'] as $property) {                    
                    $property['sq:accountId'] = $account['id'];
                    $props[$property['id']]   = $property;
                }
            }//end foreach
        }//end if
        
        return $props;
        
    }//end getProperties()


    /**
     * Gets a list of views from the properties array.
     *
     * @return array
     */
    public function getViews()
    {
        $props = $this->getProperties();
        $views = Array();
        
        foreach ($props as $propid => $prop) {
            if (array_key_exists('profiles', $view) === TRUE) {
                foreach ($prop['profiles'] as $view) {
                    $view['sq:accountId']  = $prop['sq:accountId'];
                    $view['sq:propertyId'] = $propid;
                    $views[$view['id']] = $view;
                }
            }
        }
        
        return $views;
        
    }//end getViews()
    
    
    /**
     * 
     */
    public function getGoals($viewid)
    {
        // Cache for 4 hours.
        $cm       = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key      = 'goals,'.$viewid;
        $response = Array();
        
        $refresh_key = $key.'.refresh';
        $expiry      = (4 * 60 * 60);
        
        $refresh_resp = $this->loadFromFetchCache($this->getRefreshKey($key));
        if ((string) $refresh_resp === '1') {
            // Not stale yet.
            $response = $this->loadFromFetchCache($key);
        } else {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $url      = self::FETCH_PREFIX.'/management/accounts/~all/webproperties/~all/profiles/~all/goals';
            $response = $this->fetchWithBackoff($url);
            
            if ($response !== NULL) {
                $this->saveToFetchCache($key, $response);
            } else {
                $response = $this->loadFromFetchCache($key);
            }
        }//end if
        
        if (($response === FALSE) || ($response === NULL)) {
            $goals = Array();
        } else {
            $goals = Array();
            foreach (array_get_index($response, 'items', Array()) as $goal) {
                if ($goal['profileId'] === $viewid) {
                    $goals[$goal['id']] = $goal;
                }
            }//end foreach
        }//end if
        
        return $goals;
    }//end getGoals()
    
    
    /**
     * 
     */
    public function getExperiments($viewid)
    {
        // Cache for 4 hours.
        $cm       = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key      = 'experiments,'.$viewid;
        $response = Array();
        $views    = $this->getViews();
        $view     = $views[$viewid];
        
        $refresh_key = $key.'.refresh';
        $expiry      = (4 * 60 * 60);
        
        $refresh_resp = $this->loadFromFetchCache($this->getRefreshKey($key));
        if ((string) $refresh_resp === '1') {
            // Not stale yet.
            $response = $this->loadFromFetchCache($key);
        } else {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $url      = self::FETCH_PREFIX.'/management/accounts/'.$view['sq:accountId'].'/webproperties/'.$view['sq:propertyId'].'/profiles/'.$viewid.'/experiments';
            $response = $this->fetchWithBackoff($url);
            
            if ($response !== NULL) {
                $this->saveToFetchCache($key, $response);
            } else {
                $response = $this->loadFromFetchCache($key);
            }
        }//end if
        
        if (($response === FALSE) || ($response === NULL)) {
            $experiments = Array();
        } else {
            $experiments = Array();
            foreach (array_get_index($response, 'items', Array()) as $experiment) {
                $experiments[$experiment['id']] = $experiment;
            }//end foreach
        }//end if
        
        return $experiments;
    }//end getExperiments()
    
    
    /**
     * Wrapper to saving fetched data from our Cache Manager.
     *
     * This is for our fetched data where we want it to be fresh within a
     * certain time period (eg. 4 hours) but concede that we cannot always
     * updated it. Cache Manager doesn't have the concept of "stale but
     * retrievable" data, so we need to write our own wrapper.
     *
     * So this saves two values: a key and a refresh key. The key contains the
     * value saved and has a very long-term expiry. The refresh key contains a
     * basic value (string '1') and will expire as per our fetch expiry.
     *
     * @param string $key   The (main) key to store the value under.
     * @param mixed  $value The value to store to the key.
     *
     * @return void
     */
    public function saveToFetchCache($key, $value)
    {
        // TODO: replace with attribute
        $long_expiry = (3 * 365 * 60 * 60);
        $cm          = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key         = 'fetch.'.$key;
        $refresh_key = $this->getRefreshKey($key);
        
        $cm->saveToCache($this->id, $this->type(), $key, json_encode($value), FALSE, $long_expiry);
        $cm->saveToCache($this->id, $this->type(), $refresh_key, json_encode('1'), FALSE);
        
        $this->loadFromFetchCache(substr($key, 6));
        $this->loadFromFetchCache(substr($refresh_key, 6));
        
    }//end saveToFetchCache()
    
    
    /**
     * Wrapper to loading fetched data from our Cache Manager.
     *
     * @param string $key   The (main) key to store the value under.
     *
     * @return void
     */
    public function loadFromFetchCache($key)
    {
        $key    = 'fetch.'.$key;
        $cm     = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $result = $cm->loadFromCache($this->id, $this->type(), $key, FALSE);
        return json_decode($result, TRUE);
        
    }//end loadFromFetchCache()
    
    
    /**
     * Get refresh key.
     */
    public function getRefreshKey($key)
    {
        return $key.'.refresh';
    }
    
    
    /**
     * Fetch but with Google Analytics' exponential backoff rules.
     *
     * GA's backoff rules are used when per-second quota limits are exceeded.
     * At time of writing this is 10 queries/sec per view, and a limit of
     * queries/sec per IP address (default 1, max 10). The backoff starts at
     * one second then doubles until it reaches 16 seconds (+ random partial
     * second).
     * Also implements a single backoff for 500 errors.
     *
     * Call in place of fetch(). If a fetch fails (and in certain cases,
     * retries run out), then return NULL.
     *
     * @param string $url   The URL to fetch.
     *
     * @return array
     */
    public function fetchWithBackoff($url)
    {
        if (($this->attr('client_id') === '') || ($this->attr('client_secret') === '')) {
            return NULL;
        }
        
        $result  = NULL;
        $retries = NULL;
        $code    = NULL;
        while (($retries === NULL) || ($retries > 0)) {
            $response = $this->fetch($url, FALSE);
            
            // Firstly if we get one error initially then we get another,
            // don't retry any further regardless of the error.
            if (($code !== NULL) && ($response['code'] !== $code)) {
                break;
            } else {
                $code = $response['code'];
            }
            
            switch ((int) $response['code']) {
                case 200:
                    // OK. Take the result and 
                    $result  = $response['result'];
                    $retries = 0;
                break;
                
                case 400:
                    // Bad request or invalid parameter. Cannot retry.
                    $errors = $response['result']['error']['errors'];
                    bam($errors);
                    $retries = 0;
                break;
                
                case 401:
                    // Token has been revoked.
                    $errors = $response['result']['error']['errors'];
                    bam($errors);
                    $retries = 0;
                break;
                
                case 403:
                    // Find error list. If the 403 is due to a rate-per-second
                    // limit, allow exponential backoff. Otherwise no retries.
                    $errors       = $response['result']['error']['errors'];
                    $retryBlocked = TRUE;
                    foreach ($errors as $error) {
                        if (($error['reason'] === 'userRateLimitExceeded') ||
                            ($error['reason'] === 'quotaExceeded')) {
                            $retryBlocked = FALSE;
                        } else {
                            $retryBlocked = TRUE;
                            break;
                        }
                    }//end foreach
                    
                    if ($retryBlocked === TRUE) {
                        // Retries are blocked.
                        $retries = 0;
                    } else {
                        if ($retries === NULL) {
                            // First retry.
                            $retries = 5;
                        } else {
                            // Subsequent retry.
                            $retries--;
                        }
                        
                        if ($retries > 0) {
                            // If there are retries left, back off for 1 sec,
                            // then 2 sec, 4 sec... etc. Also add a random
                            // component between 0-1 second.
                            $waitTime  = (32.0 / pow(2, (5 - $retries)));
                            $waitTime += (mt_rand() / mt_getrandmax());
                            usleep($waitTime * 1e6);
                        }
                    }//end if
                break;
                
                case 500:
                    if ($retries === NULL) {
                        // Only one retry. 1 second wait + random 0-1 second.
                        $retries   = 1;
                        $waitTime  = 1.0;
                        $waitTime += (mt_rand() / mt_getrandmax());
                        usleep($waitTime * 1e6);
                    } else {
                        // Won't be fooled again.
                        $retries--;
                    }
                break;

                default:
                    // Unexpected result, including DNS issues
                    // and 404s. Don't retry these.
                    $retries = 0;
                break;
            }
        }
        
        return $result;
        
    }//end fetchWithBackoff()
    
    
    public function revokeAccessToken()
    {
        
        
    }//end revokeAccessToken()


//--      BRIDGE FUNCTIONS        --//

	
	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* Returns NULL on error
	*
	* @param int		$assetid		the full asset ID of the shadow asset
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	deprecated
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	public function getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
	    $result = NULL;
	    list($realid, $shadowid) = explode(':', $assetid, 2); 
	    
	    $shadow_parts = explode('-', $shadowid, 2);
	    switch ($shadow_parts[0]) {
	        case 'p':
	            // Property.
	            $props = $this->getProperties();
	            if (array_key_exists($shadow_parts[1], $props)) {
                    $GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_property');
                    $result = new Google_Analytics_Property();
                    $result->id       = $this->id.':'.$shadowid;
                    $result->bridgeid = $this->id;
                    
                    $name = $props[$shadow_parts[1]]['name'];
                    $result->setAttrValue('name', $name);
                    $result->name       = $name;
                    $result->short_name = $name;
                    $result->status     = SQ_STATUS_UNDER_CONSTRUCTION;
	            }//end if
	        break;
	        
	        case 'v':
	            // View.
	            $views = $this->getViews();
	            if (array_key_exists($shadow_parts[1], $views)) {
                    $GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_view');
                    $result = new Google_Analytics_View();
                    $result->id         = $this->id.':'.$shadowid;
                    $result->viewid     = $shadow_parts[1];
                    $result->bridgeid   = $this->id;
                    $result->propertyid = $views[$shadow_parts[1]]['sq:propertyId'];
                    
                    $name = $views[$shadow_parts[1]]['name'];
                    $result->setAttrValue('name', $name);
                    $result->name       = $name;
                    $result->short_name = $name;
                    $result->status     = SQ_STATUS_UNDER_CONSTRUCTION;
                }//end if
	        break;
	    }//end switch
	    
	    return $result;
	    
	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	public function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
	    $result = Array();
	    
	    if ($assetid === $this->id) {
	        if ($side_of_link === 'major') {
	            $props = $this->getProperties();
                foreach ($props as $property) {
                    $result[] = array(
                        'linkid'          => $assetid.':p-'.$property['id'],
                        'majorid'         => $assetid,
                        'minorid'         => $assetid.':p-'.$property['id'],
                        'minor_type_code' => 'google_analytics_property',
                        'link_type'       => SQ_LINK_TYPE_2,
                        'value'           => '',
                        'sort_order'      => -1,
                        'locked'          => 1,
                        'is_dependant'    => 1,
                        'is_exclusive'    => 1,
                    );
                }//end foreach properties
	        }//end if major side
	    } else {
	        // Shadow asset.
	        list($realid, $shadowid) = explode(':', $assetid, 2);
	        $shadow_parts = explode('-', $shadowid, 2);
                    
            switch ($shadow_parts[0]) {
                case 'p':
                    // Property.
                    $props = $this->getProperties();
                    if (array_key_exists($shadow_parts[1], $props) === TRUE) {
                        $property = $props[$shadow_parts[1]];
                        if ($side_of_link === 'major') {
                            $views = $this->getViews();
                            foreach ($views as $view) {
                                if ($view['sq:propertyId'] === $property['id']) {
                                $result[] = array(
                                    'linkid'          => $realid.':v-'.$view['id'],
                                    'majorid'         => $assetid,
                                    'minorid'         => $realid.':v-'.$view['id'],
                                    'minor_type_code' => 'google_analytics_view',
                                    'link_type'       => SQ_LINK_TYPE_2,
                                    'value'           => '',
                                    'sort_order'      => -1,
                                    'locked'          => 1,
                                    'is_dependant'    => 1,
                                    'is_exclusive'    => 1,
                                );
                                }
                            }//end foreach properties
                        } else {
                            $result[] = array(
                                'linkid'          => $assetid.':p-'.$property['id'],
                                'majorid'         => $realid,
                                'minorid'         => $assetid,
                                'major_type_code' => 'google_analytics_bridge',
                                'link_type'       => SQ_LINK_TYPE_2,
                                'value'           => '',
                                'sort_order'      => -1,
                                'locked'          => 1,
                                'is_dependant'    => 1,
                                'is_exclusive'    => 1,
                            );
                        }//end if major side
                    }//end if property exists
                break;
                
                case 'v':
                    // View.
                    $views = $this->getViews();                        
                    if (array_key_exists($shadow_parts[1], $views) === TRUE) {                            
                        $view = $views[$shadow_parts[1]];
                        if ($side_of_link === 'major') {
                            // No major links yet, TODO: goals and experiments
                        } else {
                            $result[] = array(
                                'linkid'          => $assetid.':v-'.$view['id'],
                                'majorid'         => $realid.':p-'.$view['sq:propertyId'],
                                'minorid'         => $assetid,
                                'major_type_code' => 'google_analytics_property',
                                'link_type'       => SQ_LINK_TYPE_2,
                                'value'           => '',
                                'sort_order'      => -1,
                                'locked'          => 1,
                                'is_dependant'    => 1,
                                'is_exclusive'    => 1,
                            );
                        }//end if major side
                    }//end if view exists
                break;
            }//end switch
	    }//end if
	    
	    return $result;
	    
	}//end getLinks()
	
	
	/**
	 *
	 */
	public function getLink($assetid, $link_type, $type_code='', $strict_type_code=TRUE, $value='', $side_of_link='major', $exclusive=NULL)
	{
	    $result = Array();
	    $links = $this->getLinks($assetid, $link_type, $type_code, $strict_type_code, $side_of_link);
	    if (count($links) > 0) {
	        $result = $links[0];
	    }
	    
	    return $result;
	    
	}//end getLink()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int		$assetid			the id of the asset to get its parents for
	* @param mixed		$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*										if an array returns link if matches any of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	public function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* Returns an array of assetids and their type code
	*
	* @param int		$assetid			the id of the asset to get its children for
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param boolean	$dependant			if TRUE, results will be filtered to assets that are dependants.
	*										If FALSE, results will all not be dependants.
	*										If NULL, results will not be filtered
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	public function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	public function getLineageFromURL($assetid, $protocol, $url)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getLineageFromURL()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc) (can be a string or an array of strings)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link id when returning the count
	*
	* @return int
	* @access public
	*/
	public function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
	    $result = 0;
	    
	    return $result;
	    
	}//end countLinks()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	public function deleteAssetLink($linkid, $moving=FALSE)
	{
	    $ok = FALSE;
	    
	    return $ok;
	    
	}//end deleteAssetLink()


	/**
	* Get asset info for use by asset map
	*
	* Should return an array containing the following elements:<ul>
	* 	<li>assetid:    ID of the asset.</li>
	* 	<li>name:       Full name of the asset.</li>
	*	<li>short_name: Short name of the asset.</li>
	*	<li>version:    Asset version number.<li>
	*	<li>status:     Current status of the asset.</li>
	*	<li>type_code:  The type code of the shadow asset.</li>
	*   <li>num_kids:   Number of children, if known.</li>
	*	<li>accessible: Whether the asset should be accessible.</li>
	*	<li>url:        URL for this lineage of the asset, if known.</li>
	*	<li>web_path:   Web path for this lineage of the asset, if known.</li>
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	public function getAssetMapAssetInfo($assetid)
	{
	    $result = Array();
	    
	    return $result;
	    
	}//end getAssetMapAssetInfo()


}//end class

?>
