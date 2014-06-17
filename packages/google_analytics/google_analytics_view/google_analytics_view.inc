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

	
	
	
	/**
	 * Generate the visitors report for this property
	 *
	 * Returns an array of the appropriate data, that can be sent to keywords
	 * and reports, both front-end and back-end (edit interface).
	 *
	 * @return array
	 */
	public function generateVisitorsReport($dimensions=Array(), $start_date, $end_date)
	{
	    require_once SQ_FUDGE_PATH.'/general/www.inc';
	    
	    $GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_bridge');
	    $bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->bridgeid);
	    
	    // Cache for 4 hours.
        $cache_key = 'visitors';
        
        list($realid, $shadowid) = explode(':', $this->id, 2);
        list(, $viewid) = explode('-', $shadowid, 2);
        
        // Query definitions.
        $common_query_parts = Array(
            'ids'         => 'ga:'.$viewid,
            'start-date'  => date('Y-m-d', $start_date),
            'end-date'    => date('Y-m-d', $end_date),
            'max-results' => 10000,
            'start-index' => 1,
            'fields'      => 'rows',
        );
        
        $queries = Array(
            'summary' => Array(
                'dimensions' => $dimensions,
                'metrics'    => Array(
                    'ga:sessions',
                    'ga:newUsers',
                    'ga:users',
                    'ga:pageViews',
                    'ga:sessionDuration',
                    'ga:bounces'
                ),
            ),
        );
        
        $params = array_merge(
            $queries['summary'],
            $common_query_parts
        );
        
        $cache_key .= '.'.http_build_query($params, '', ',');
        
        $refresh_resp = $bridge->loadFromFetchCache($bridge->getRefreshKey($cache_key));
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
            if (array_key_exists('sort', $url_params) === TRUE) {
                $url_params['sort'] = implode(',', $url_params['sort']);
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
            $response = $bridge->fetchWithBackoff($url);
            
            if ($response !== NULL) {
                $bridge->saveToFetchCache($cache_key, $response);
            } else {
                $response = $bridge->loadFromFetchCache($cache_key);
            }
        }//end if
        
        if (($response === FALSE) || ($response === NULL)) {
            $result = Array();
        } else {
            $result = $response['rows'];
            foreach ($result as &$row) {
                $row = array_combine(($params['dimensions'] + $params['metrics']), $row);
            }
        }//end if
        
        return $result;
	    
	}//end generateVisitorsReport()
	
	


}//end class

?>
