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


require_once SQ_INCLUDE_PATH.'/asset.inc';
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
class Google_Analytics_View extends Asset
{
    /**
     * Common site-level metrics.
     */
    const SITE_METRICS = 'ga:sessions,ga:newUsers,ga:users,ga:pageviews,ga:sessionDuration,ga:bounces,ga:goalStartsAll,ga:goalCompletionsAll';
    
    /**
     * Common page-level metrics.
     */
    const PAGE_METRICS = 'ga:pageviews,ga:uniquePageviews,ga:entrances,ga:exits,ga:bounces';
    
    
    /**
     * Shortcut to access the view ID (without ga: prefix).
     *
     * @var string
     */
    var $viewid = NULL;
    
    /**
     * Shortcut to access the bridge asset
     *
     * @var string
     */
    var $bridgeid = NULL;
    
    
    /**
     * Property ID
     *
     * Just the string provided by GA. Convert to a full asset ID using
     * $prop_assetid = $this->bridgeid.':p-'.$this->propertyid; 
     *
     * @var string
     */
    var $propertyid = NULL;
    
    
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

	
	
	protected function _getName($short_name=FALSE)
	{
	    return $this->name;
	    
	}//end _getName()
	
	
	/**
	 * Get keyword replacement
	 *
	 * @param string $keyword_name The keyword to fetch
	 *
	 * @return string
	 */
	public function getKeywordReplacement($keyword_name)
	{
	    $keyword_repl = '';
	    
	    return $keyword_repl; 
	    
	}//end getKeywordReplacement()
	
	
	
	/**
	 * Generate the visitors report for this property
	 *
	 * Returns an array of the appropriate data, that can be sent to keywords
	 * and reports, both front-end and back-end (edit interface).
	 *
	 * @return array|NULL
	 */
	public function generateReport($dimensions, $metrics, $start_date, $end_date, $sort=Array(), $filter=Array(), $force=FALSE)
	{
	    require_once SQ_FUDGE_PATH.'/general/www.inc';
	    
	    $GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_bridge');
	    $bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->bridgeid);
	    
	    // Cache for 4 hours.
        $cache_key = 'visitors';
        
        if (is_string($start_date) === FALSE) {
            $start_date = date('Y-m-d', $start_date);
        }
        
        if (is_string($end_date) === FALSE) {
            $end_date = date('Y-m-d', $end_date);
        } else if (strpos($end_date, '-P') === 0) {
            // We got given a negative duration in P format.
            $period     = substr($end_date, 1);
            $end_date   = new DateTime('today 00:00:00');
            $end_date->sub(new DateInterval($period));
            $end_date   = $end_date->format('Y-m-d');
        } else if (strpos($end_date, 'P') === 0) {
            // We got given a duration in P format.
            $period     = $end_date;
            $end_date   = new DateTime('today 00:00:00');
            $end_date->add(new DateInterval($period));
            $end_date   = $end_date->format('Y-m-d');
        }
        
        // Query definitions.
        $common_query_parts = Array(
            'ids'         => 'ga:'.$this->viewid,
            'start-date'  => $start_date,
            'end-date'    => $end_date,
            'max-results' => 10000,
            'start-index' => 1,
            'fields'      => 'rows',
        );
        
        if (is_string($dimensions)) {
            $dimensions = explode(',', $dimensions);
        }
        
        if (is_string($metrics)) {
            $metrics = explode(',', $metrics);
        }
        
        $query = Array(
            'dimensions' => $dimensions,
            'metrics'    => $metrics,
            'sort'       => $sort,          
        );
        
        // Add the filter.
        if (is_array($filter) === TRUE) {
            $query['filters'] = $filter;
        }
        
        $params = array_merge(
            $query,
            $common_query_parts
        );
 
        $cache_key .= '.'.http_build_query($params, '', ',');
        
        if ($force === TRUE) {
            $refresh_resp = '0';
        } else {
            $refresh_resp = $bridge->loadFromFetchCache($bridge->getRefreshKey($cache_key));
        }
        
        if ((string) $refresh_resp === '1') {
            // Not stale yet.
            $response = $bridge->loadFromFetchCache($cache_key);
        } else {
            // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
            // back to any previously existing cache.
            $url      = Google_Analytics_Bridge::FETCH_PREFIX.'/data/ga';
            
            // Flatten out the dimensions and metrics.
            $url_params = $params;
            $url_params['dimensions'] = implode(',', $url_params['dimensions']);
            $url_params['metrics']    = implode(',', $url_params['metrics']);
            
            // Also sorting if specified.
            if (count($url_params['sort']) > 0) {
                $url_params['sort'] = implode(',', $url_params['sort']);
            } else {
                unset($url_params['sort']);
            }
            
            if (array_key_exists('filters', $url_params) === TRUE) {
                foreach ($url_params['filters'] as &$item) {
                    // Filters can be two-dimensional, but the insides are always
                    // "OR"d together.
                    if (is_array($item) === TRUE) {
                        $item = implode(',', $item);
                    }
                }
                
                // Then they are "AND"ed.
                $url_params['filters'] = implode(';', $url_params['filters']);
            }
            
            $url = replace_query_string_vars($url_params, $url);
            $raw_response = $bridge->fetchWithBackoff($url);
            
            if ($raw_response['errors'] === NULL) {
                $response = $raw_response['result'];
                $bridge->saveToFetchCache($cache_key, $response);
            } else {
                $response = $bridge->loadFromFetchCache($cache_key);
            }
        }//end if
        
        if (($response === FALSE) || ($response === NULL)) {
            $result = Array(
                        'errors' => $raw_response['errors'],
                      );
        } else {
            $result = array_get_index($response, 'rows', Array());
            foreach ($result as &$row) {
                $row = array_combine(array_merge($params['dimensions'], $params['metrics']), $row);
                
                // Turn metrics into numbers
                foreach ($params['metrics'] as $metric) {
                    if (strpos($row[$metric], '.') === FALSE) {
                        $row[$metric] = (int) $row[$metric];
                    } else {
                        $row[$metric] = (float) $row[$metric];
                    }
                }
            }
        }//end if
        
        return $result;
	    
	}//end generateReport()


}//end class

?>
