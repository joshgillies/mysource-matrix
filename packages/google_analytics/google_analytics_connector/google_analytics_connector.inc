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
*/


require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token.inc';

/**
* Google Analytics Connector
*
* Purpose
*
*
* @author  Squiz Pty Ltd <products@squiz.net>
* @package MySource_Matrix_Packages
* @subpackage google_analytics
*/
class Google_Analytics_Connector extends OAuth2_Token
{


    /**
     * The authentication endpoint.
     *
     * The "access_type=offline" query param is to ensure we get a refresh
     * token. Google doesn't provide this by default.
     *
     * The "approval_prompt=force" query param is also for refresh tokens.
     * Google doesn't give out refresh tokens without showing the user a
     * consent screen (and fair enough) and this is therefore necessary if
     * a project has to be re-authed on an account for some reason.
     *
     * @const string
     */
    const AUTH_ENDPOINT = 'https://accounts.google.com/o/oauth2/auth?access_type=offline&approval_prompt=force';
    
    /**
     * The token endpoint.
     *
     * @const string
     */
    const TOKEN_ENDPOINT = 'https://accounts.google.com/o/oauth2/token';
    
    /**
     * Space-separated list of scopes desired by this connector.
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
        $this->setAttrValue('access_token_type', 'bearer');
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
		$links[SQ_LINK_TYPE_1]['google_analytics_view'] = Array('card' => 'M', 'exclusive' => FALSE);
        $links[SQ_LINK_NOTICE]['google_analytics_view'] = Array('card' => 'M', 'exclusive' => FALSE);

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
     * Get the web properties available to this account.
     *
     * @return array
     */
    public function getAccountSummary()
    {
        // Cache for 4 hours.
        $cm       = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key      = 'accountSummaries';
        $response = $this->loadFromFetchCache($key);
        if ($response === NULL) {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $url = self::FETCH_PREFIX.'/management/accountSummaries';
            $raw_response = $this->fetchWithBackoff($url);
            
            if ($raw_response['errors'] === NULL) {
                $response = $raw_response['result'];
                $this->saveToFetchCache($key, $response);
            } else {
                $response = $this->loadFromFetchCache($key);
            }
        }//end if

        return $response;
        
    }//end getProperties()


    /**
     * Get the web properties available to this account.
     *
     * @return array
     */
    public function getProperties()
    {
        $response = $this->getAccountSummary();

		$props = Array();
        if (!empty($response)) {
            foreach (array_get_index($response, 'items', Array()) as $account) {
                foreach (array_get_index($account, 'webProperties', Array()) as $property) {
                    if (array_key_exists('profiles', $property) === TRUE) {
                        $property['sq:accountId'] = $account['id'];
                        $props[$property['id']]   = $property;
                    }
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
            if (array_key_exists('profiles', $prop) === TRUE) {
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
     * Return a list of goals from the Google Analytics API.
     *
     * The unique ID sent should not include the "ga:" prefix.
     *
     * @param string $gaViewId    The Google Analytics unique ID of the view.
     *
     * @return mixed array|NULL
     */
    public function getViewDetails($gaViewId)
    {
        // Cache for 4 hours.
        $cm       = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key      = 'view,'.$gaViewId;
        $response = $this->loadFromFetchCache($key);
        if ($response === NULL) {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $views = $this->getViews();
            $viewSummary = $views[$gaViewId];
            $url = self::FETCH_PREFIX.'/management/accounts/'.$viewSummary['sq:accountId'].'/webproperties/'.$viewSummary['sq:propertyId'].'/profiles/'.$gaViewId;
            $raw_response = $this->fetchWithBackoff($url);
            
            if ($raw_response['errors'] === NULL) {
                $response = $raw_response['result'];
                $this->saveToFetchCache($key, $response);
            } else {
                $response = $this->loadFromFetchCache($key);
            }
        }//end if
        
        return $response;

    }//end getViewDetails()
    
    
    /**
     * Return a list of goals from the Google Analytics API.
     *
     * The unique ID sent should not include the "ga:" prefix.
     *
     * @param string $gaViewId    The Google Analytics unique ID of the view.
     *
     * @return mixed array|NULL
     */
    public function getGoals($gaViewId)
    {
        // Cache for 4 hours.
        $cm       = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key      = 'goals,'.$gaViewId;
        $response = $this->loadFromFetchCache($key);
        if ($response === NULL) {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $url = self::FETCH_PREFIX.'/management/accounts/~all/webproperties/~all/profiles/~all/goals';
            $raw_response = $this->fetchWithBackoff($url);
            
            if ($raw_response['errors'] === NULL) {
                $response = $raw_response['result'];
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
                if ($goal['profileId'] === $gaViewId) {
                    $goals[] = $goal;
                }
            }//end foreach
        }//end if
        
        return $goals;
    }//end getGoals()
    
    
    /**
     * Return a list of experiments from the Google Analytics API.
     *
     * The unique ID sent should not include the "ga:" prefix.
     *
     * @param string $gaViewId    The Google Analytics unique ID of the view.
     *
     * @return mixed array|NULL
     */
    public function getExperiments($gaViewId)
    {
        // Cache for 4 hours.
        $cm       = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
        $key      = 'experiments,'.$gaViewId;
        $views    = $this->getViews();
        $view     = $views[$gaViewId];
        $response = $this->loadFromFetchCache($key);
        if ($response === NULL) {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $url = self::FETCH_PREFIX.'/management/accounts/'.$view['sq:accountId'].'/webproperties/'.$view['sq:propertyId'].'/profiles/'.$gaViewId.'/experiments';
            $raw_response = $this->fetchWithBackoff($url);
            
            if ($raw_response['errors'] === NULL) {
                $response = $raw_response['result'];
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
                $experiments[] = $experiment;
            }//end foreach
        }//end if
        
        return $experiments;
    }//end getExperiments()
    
    
    /**
     * Wrapper to saving fetched data to our fetch cache.
     *
     * We can't use Cache Manager as that's meant for a different
     * type of caching, and can't be used for a "global cache".
     *
     * The suffix parameter should be used when creating a temporary
     * cache during cron generation, so that the existing data doesn't
     * get mixed in with the day-old data. Afterwards, replaceFetchCache()
     * should be called, to copy in the new data.
     * When getting data on the fly, data should be refreshed
     * immediately, so should be run over the lot.
     *
     * @param string $key    The (main) key to store the value under.
     * @param mixed  $value  The value to store to the key.
     * @param string $suffix Suffix for a temporary "working" cache.
     *
     * @return void
     */
    public function saveToFetchCache($key, $value, $suffix='')
    {
        include_once SQ_FUDGE_PATH.'/general/file_system.inc';
        $file = $this->getFetchCacheFileName($key);
        if ($suffix !== '') {
            $file .= '.'.$suffix;
        }
        
        create_directory(dirname($file));
        file_put_contents($file, json_encode($value));
        
    }//end saveToFetchCache()
    
    
    /**
     * Replace fetch cache with a suffixed one.
     *
     * When we generate the cache in the cron job, we use a temporary
     * cache so that the existing data doesn't get mixed in with the
     * day-old data. At the end of each view's work, this is called to
     * copy the new data over the old.
     *
     * @param string $suffix Cache suffix to use. Do not include ".".
     *
     * @return void
     */
    public function replaceFetchCache($suffix='')
    {
        if ($suffix === '') {
            return;
        }

        include_once SQ_FUDGE_PATH.'/general/file_system.inc';
        ob_start();
        system('find '.escapeshellarg($this->data_path).' -name '.escapeshellarg('*.'.$suffix));
        $files = ob_get_clean();

        $files = explode("\n", $files);
        foreach ($files as $file) {
            if (empty($file) === FALSE) {
                $basefile = dirname($file).'/'.basename($file, '.'.$suffix);
                $ok = copy($file, $basefile);
                if ($ok === TRUE) {
                    unlink($file);
                }
            }
        }
        
    }//end replaceFetchCache()
    
    /**
     * Wrapper to loading fetched data from our own cache.
     *
     * @param string  $key    The (main) key to store the value under.
     * @param integer $expiry The length of time the cache stays valid (default 4 hours).
     *
     * @return integer
     */
    public function getFetchCacheAge($key)
    {
        $file = $this->getFetchCacheFileName($key);

        if (file_exists($file) === FALSE) {
            return NULL;
        } else {
            $filetime = filemtime($file);
            $seconds  = (time() - $filetime);
            return $seconds;
        }
        
    }//end getFetchCacheAge()
    
    
    /**
     * Wrapper to loading fetched data from our own cache.
     *
     * @param string  $key    The (main) key to store the value under.
     * @param integer $expiry The length of time the cache stays valid (default 4 hours).
     *
     * @return void
     */
    public function loadFromFetchCache($key, $expiry=14400)
    {
        $file = $this->getFetchCacheFileName($key);

        if (file_exists($file) === FALSE) {
            return NULL;
        } else if (($expiry > 0) && ((time() - filemtime($file)) > $expiry)) {
            unlink($file);
            return NULL;
        }     

        $result = file_get_contents($file);
        return json_decode($result, TRUE);
        
    }//end loadFromFetchCache()
    
    
    /**
     * Returns a hashed file name location, given a cache key.
     *
     * Cache keys are SHA-1'd and stored in the location
     * (asset data path)/fetch-cache/(first 2 hex-digits of hash)/(hash).json.
     * eg. fetch-cache/ab/abc123def456[...]ef0.json.
     *
     * @param string $key   The applicable cache key.
     *
     * @return string
     */
    public function getFetchCacheFileName($key)
    {
        $key  = sha1($key);
        $dir  = $this->data_path.'/fetch-cache/'.substr($key, 0, 2).'/'.substr($key, 2, 2);
        $file = $dir.'/'.$key.'.json';

        return $file;
        
    }//end getFetchCacheFileName()
    
    
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
        $errors  = NULL;
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
                    $retries = 0;
                break;
                
                case 401:
                    // Token has been revoked.
                    $errors = $response['result']['error']['errors'];
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
        
        $retval = Array(
                    'result' => $result,
                    'code'   => $code,
                    'errors' => $errors,
                  );
        
        return $retval;
        
    }//end fetchWithBackoff()


}//end class

?>
