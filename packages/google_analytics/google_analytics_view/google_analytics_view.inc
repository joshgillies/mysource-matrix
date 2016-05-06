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


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Google Analytics View
*
* Purpose
*
*
* @author  Squiz Pty Ltd <products@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage google_analytics
*/
class Google_Analytics_View extends Page
{
    /**
     * Common site-level metrics.
     */
    const SITE_METRICS = 'ga:sessions,ga:newUsers,ga:users,ga:pageviews,ga:sessionDuration,ga:bounces,ga:pageLoadTime,ga:pageLoadSample,ga:goalStartsAll,ga:goalCompletionsAll';

    /**
     * Common page-level metrics.
     */
    const PAGE_METRICS = 'ga:pageviews,ga:uniquePageviews,ga:entrances,ga:exits,ga:bounces';


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
	    return $this->attr('name');

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
	 * Filter enforces the logic permitted by Google Analytics. At the first
	 * level of the array, the options there are ANDed. If any of the elements
	 * is itself an array, they are first ORd together before becoming one AND
	 * option.
     *
     * Force cache setting is a tri-state setting:
     * FALSE (or NULL) means: use the cache and fetch data if it
     * doesn't exist.
     * TRUE (or 'cache') means use the cache and return NULL if
     * nothing exists in the cache.
     * 'nocache' means do not use the cache at all, always fetch
     * data. Canned queries use this as they cache their own data.
	 *
	 * Returns array if successful or error. If nothing in the cache and
	 * $force_cache is TRUE, return NULL instead.
	 *
	 * @param array   $dimensions    List of dimensions.
	 * @param array   $metrics       List of metrics.
	 * @param mixed   $start_date    Start date (Y-m-d or unix timestamp).
	 * @param string  $end_date      End date (Y-m-d or unix timestamp).
	 * @param array   $sort          Sort order.
	 * @param array   $filter        Filters.
     * @param array   $segment       Segments.
	 * @param mixed   $force_cache   Force cache setting.
	 *
	 * @return array|NULL
	 */
	public function generateReport($dimensions, $metrics, $start_date, $end_date, $sort=Array(), $filter=Array(), $segment=Array(), $force_cache=FALSE)
	{
	    require_once SQ_FUDGE_PATH.'/general/www.inc';

	    $GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_connector');
	    $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'google_analytics_connector', FALSE, NULL, 'minor');
	    $connector      = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['majorid']);

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

        // Recalculate the period.
        $start_obj = new DateTime($start_date);
        $period    = $start_obj->diff(new DateTime($end_date), TRUE);

        // Query definitions.
        $common_query_parts = Array(
            'ids'         => 'ga:'.$this->attr('viewid'),
            'start-date'  => $start_date,
            'end-date'    => $end_date,
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

        // Add the final result filter if it exists.
        if (is_array($filter) === TRUE) {
            $query['filters'] = $filter;
        }

        // Add the segmentation filter if it exists.
        if (is_array($segment) === TRUE) {
            $query['segment'] = $segment;
        }

        $params = array_merge(
            $query,
            $common_query_parts
        );

        // Build cache key and 4-hour expiry.
        $cache_key .= '.'.http_build_query($params, '', ',');
        $expiry     = (4 * 60 * 60);

        $response = NULL;
        if ($force_cache !== 'nocache') {
            $response = $connector->loadFromFetchCache($cache_key, $expiry);
        }

        if (($response === NULL) && (($force_cache !== TRUE) && ($force_cache !== 'cache'))) {
            $done     = FALSE;
            $response = Array();
            $url_params = $params;
            $url_params['start-index'] = 1;
            $url_params['max-results'] = 10000;

            // Flatten out the dimensions and metrics.
            $url_params['dimensions'] = implode(',', $url_params['dimensions']);
            $url_params['metrics']    = implode(',', $url_params['metrics']);

            // Also sorting if specified.
            if (count($url_params['sort']) > 0) {
                $url_params['sort'] = implode(',', $url_params['sort']);
            } else {
                unset($url_params['sort']);
            }

            // Final result filters.
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

            // Segment filters are treated the same way.
            if (array_key_exists('segment', $url_params) === TRUE) {
                foreach ($url_params['segment'] as &$item) {
                    // Filters can be two-dimensional, but the insides are always
                    // "OR"d together.
                    if (is_array($item) === TRUE) {
                        $item = implode(',', $item);
                    }
                }

                // Then they are "AND"ed.
                $url_params['segment'] = implode(';', $url_params['segment']);
            }

            if (array_get_index($url_params, 'filters', '') === '') {
                unset($url_params['filters']);
            }

            if (array_get_index($url_params, 'segment', '') === '') {
                unset($url_params['segment']);
            }

            if (array_get_index($url_params, 'sort', '') === '') {
                unset($url_params['sort']);
            }

            while ($done === FALSE) {
                // Stale or doesn't exist. Fetch it, and if it doesn't work, fall
                // back to any previously existing cache.
                $url = Google_Analytics_Connector::FETCH_PREFIX.'/data/ga';

                $url = replace_query_string_vars($url_params, $url);
                $raw_response = $connector->fetchWithBackoff($url);

                if ($raw_response['errors'] === NULL) {
                    if ($response === NULL) {
                      $response = $raw_response['result'];
                    } else {
                      $response = array_merge($response, $raw_response['result']);
                    }
                    if (count($raw_response['result']) === $url_params['max-results']) {
                        // We have more results to come.
                        $url_params['start-index'] += $url_params['max-results'];
                    } else {
                        $done = TRUE;
                    }
                } else {
                    $response = NULL;
                    $done     = TRUE;
                }
            }//end while (not done)

            if (($response !== NULL) && ($force_cache !== 'nocache')) {
                $connector->saveToFetchCache($cache_key, $response);
            }
        }//end if (not cached)

        if (($response === FALSE) || ($response === NULL)) {
            if ($raw_response['errors']) {
                $result = Array(
                            'errors' => $raw_response['errors'],
                          );
            } else {
                $result = NULL;
            }
        } else {
            $result = Array(
                'start-date' => $start_date,
                'end-date'   => $end_date,
                'period'     => ($period->days + 1),
                'rows'       => array_get_index($response, 'rows', Array()),
            );
            foreach ($result['rows'] as &$row) {
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


    /**
     * Get the details of the view/profile.
     *
     * @return array|NULL
     */
    public function getView()
    {
        $connector = $this->getConnector();
        if ($connector === NULL) {
            return NULL;
        }
        
        return $connector->getViewDetails($this->attr('viewid'));

    }//end getView()


    /**
     * Get a list of experiments.
     *
     * @param boolean $force_cache   TRUE forces use of cache and won't fetch.
     *
     * @return array|NULL
     */
    public function getExperiments()
    {
        $connector = $this->getConnector();
        if ($connector === NULL) {
            return NULL;
        }

        return $connector->getExperiments($this->attr('viewid'));

    }//end getExperiments()


    /**
     * Get a list of goals.
     *
     * @param boolean $force_cache   TRUE forces use of cache and won't fetch.
     *
     * @return array|NULL
     */
    public function getGoals()
    {
        $connector = $this->getConnector();
        if ($connector === NULL) {
            return NULL;
        }

        return $connector->getGoals($this->attr('viewid'));

    }//end getGoals()


    /**
     * Get the asset ID of this view's connector.
     *
     * @return Google_Analytics_Connector|NULL
     */
    public function getConnector()
    {
        $GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_connector');
        $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'google_analytics_connector', FALSE, NULL, 'minor');
        if (empty($connector_link) === TRUE) {
            $connector = NULL;
        } else {
            $connector = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['majorid']);
        }

        return $connector;

    }//end getConnector()


    /** 
     * Get a pre-defined ('canned') report.
     *
     * Arguments array depend on the query, but these are
     * the common ones:
     * - context: the target of the report.
     *            all - ALL visits within the view
     *            site - visits underneath a certain site
     *            url - visits to a certain URL
     *            url+index - as for URL, but if that asset is also an
     *            index page, use the site's closest URL.
     *            goal - visits where a goal was converted at least once
     *            not-goal - visits where a goal was NOT converted
     *            experiment - a certain content experiment
     * - period:  the period to be obtained. The period runs up to
     *            and including "yesterday", our server time.
     *
     * These queries will be -separately- cached using the query ID and
     * canonicalised arguments, in form: queryid/arg1=val1/arg2=val2.
     * By "canonicalised": the argument keys are sorted first.
     * Note no dates in this key. This is intentional: this is so it
     * won't go for more data even if the date changes.
     *
     * Use cache setting of FALSE will request data and cache as appropriate.
     * If use cache is TRUE, it will try to use the cache. If no cache is
     * available, the age and rows elements will be NULL.
     *
     * @param string  $queryid     Pre-canned query ID.
     * @param array   $args        Optional variables depending on the query.
     * @param boolean $use_cache   Use the cached data.
     * @param string  $temp_suffix use a temporary suffix when SAVING to cache.
     *
     * @return array|NULL
     */
    public function getCannedReport($queryid, $args=Array(), $use_cache=FALSE, $temp_suffix='')
    {
        $GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_connector');
        $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'google_analytics_connector', FALSE, NULL, 'minor');
        $connector      = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['majorid']);

        // Build the cache key, including canonicalising the keys.
        $cache_key = $queryid;
        ksort($args);
        foreach ($args as $key => $val) {
            $cache_key .= '/'.$key.'='.$val;
        }

        $period  = array_get_index($args, 'duration', 'P1M');
        $context = array_get_index($args, 'context', 'all');
        $start_period = array_get_index($args, 'start_period', 'current');

        $end_date = new DateTime();
        $end_date->sub(new DateInterval('P1D'));

        $start_date = new DateTime();
        $start_date->sub(new DateInterval($period));
        if ($start_period === 'previous') {
            $start_date->sub(new DateInterval($period));
            $end_date->sub(new DateInterval($period));
        }

        // Create our basic query.
        $query = Array(
            'dimensions' => Array(),
            'metrics'    => Array(
                'ga:pageviews',
                'ga:timeOnPage',
                'ga:sessionDuration',
                'ga:entrances',
                'ga:exits',
                'ga:bounces',
                'ga:pageLoadTime',
                'ga:pageLoadSample',
                'ga:users'
            ),
            'start_date' => $start_date->format('Y-m-d'),
            'end_date'   => $end_date->format('Y-m-d'),
            'sort'       => Array(),
            'filter'     => Array(),
            'segment'    => Array(),
        );

        // Add to the base query depending on what context we want.
        switch ($context) {
            case 'site':
                $query['metrics'][] = 'ga:sessions';
                $query['sort'][] = '-ga:sessions';

                $url    = array_get_index($args, 'url');
                $scheme = NULL;
                $scheme_pos = strpos($url, '://');
                if ($scheme_pos !== FALSE) {
                    $scheme = substr($url, 0, $scheme_pos);
                    $url    = substr($url, $scheme_pos + 3);
                }

                $lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL($scheme, rtrim($url, '/'), TRUE);
                if (empty($lineage) === FALSE) {
                    $url = $lineage[0]['url'];
                }
                    
                $hostname = '';
                $pagepath = '';

                $slash_pos = strpos($url, '/');
                if ($slash_pos === FALSE) {
                    $hostname = $url;
                    $pagepath = '/';
                } else {
                    $hostname = substr($url, 0, $slash_pos);
                    $pagepath = rtrim(substr($url, $slash_pos), '/');
                    if ($pagepath === '') {
                        $pagepath = '/';
                    }
                }

                $query['filter'][] = 'ga:hostname=='.$hostname;
                if ($pagepath !== '/') {
                    $query['filter'][] = Array(
                        'ga:pagePath=='.$pagepath,
                        'ga:pagePath=~^'.$pagepath.'/',
                    );
                }
            break;

            case 'url':
            case 'url+index':
                $query['metrics'][] = 'ga:uniquePageviews';
                $query['sort'][] = '-ga:uniquePageviews';

                $url = array_get_index($args, 'url');
                $hostname   = '';
                $pagepath   = $url;
                $pageFilter = Array();

                $scheme     = NULL;
                $scheme_pos = strpos($url, '://');
                if ($scheme_pos !== FALSE) {
                    $scheme = substr($url, 0, $scheme_pos);
                    $url    = substr($url, $scheme_pos + 3);
                }

                $slash_pos = strpos($url, '/');
                if ($slash_pos === FALSE) {
                    $hostname = $pagepath;
                    $pagepath = '/';
                } else {
                    $hostname = substr($url, 0, $slash_pos);
                    $pagepath = rtrim(substr($url, $slash_pos), '/');
                    if ($pagepath === '') {
                        $pagepath = '/';
                    }
                }

                if ($context === 'url+index') {
                    // Also work out the index page, if any.
                    $asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($scheme, rtrim($url, '/'), TRUE);
                    if ($asset !== NULL) {
                        // We could be the index page to multiple sites, so go find all of
                        // them and find a URL match among them. (See SM#7473.)
                        $index_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'site', FALSE, 'minor', 'index');
                        if (count($index_links) > 0) {
                            $site_url = NULL;

                            foreach ($index_links as $index_link) {
                                $site_assetid = $index_link['majorid'];
                                $site_urls    = $GLOBALS['SQ_SYSTEM']->am->getURLs($site_assetid);

                                foreach ($site_urls as $this_url) {
                                    if (strpos($url, $this_url['url']) === 0) {
                                        // Accept the site URL if it's the first match, or if this
                                        // is a closer match than before (to take into account
                                        // sites that overlap each others' URL space).
                                        if (($site_url === NULL) || (strlen($this_url['url']) > strlen($site_url))) {
                                            $site_url = $this_url['url'];
                                        }
                                    }
                                }//end foreach site urls

                            }//end foreach links

                            if ($site_url !== NULL) {
                                $slash_pos = strpos($site_url, '/');
                                if ($slash_pos === FALSE) {
                                    $site_pagepath = '/';
                                } else {
                                    $site_pagepath = rtrim(substr($site_url, $slash_pos), '/');
                                    if ($site_pagepath === '') {
                                        $site_pagepath = '/';
                                    } else {
                                        $site_pagepath .= '(/?)';
                                    }
                                }

                                // The maximum length of a fully supported page path is 114 characters
                                // because regexes can't be more than 128 characters. The 118 value
                                // includes the trailing slash wildcard.
                                if (strlen($site_pagepath) < 118) {
                                    $pageFilter[] = 'ga:pagePath=~^'.$site_pagepath.'([?].*)?$';
                                } else {
                                    $site_pagepath = substr($site_pagepath, 0, -4);
                                    $pageFilter[] = 'ga:pagePath=='.$site_pagepath;
                                    $pageFilter[] = 'ga:pagePath=='.$site_pagepath.'/';
                                }
                            }//end if (a site URL was found)
                        }//end if (index page found)
                    }//end if (asset not found)
                }//end if (context includes index)

                // The maximum length of a fully supported page path is 114 characters
                // because regexes can't be more than 128 characters. The 118 value
                // includes the trailing slash wildcard.
                // If more than that, this will regress to an exact match, matching both
                // trailing slash and without, but unable to match use of query strings.
                if ($pagepath === '') {
                    $pagepath = '/';
                } else {
                    $pagepath .= '(/?)';
                }

                if (strlen($pagepath) < 118) {
                    $pageFilter[] = 'ga:pagePath=~^'.$pagepath.'([?].*)?$';
                } else {
                    $pagepath = substr($pagepath, 0, -4);
                    $pageFilter[] = 'ga:pagePath=='.$pagepath;
                    $pageFilter[] = 'ga:pagePath=='.$pagepath.'/';
                }

                $query['filter'][] = 'ga:hostname=='.$hostname;
                $query['filter'][] = $pageFilter;
            break;

            case 'goal':
            case 'not-goal':
                // Remove users because otherwise we have too many metrics
                $rem_index = array_search('ga:users', $query['metrics']);
                if ($rem_index !== FALSE) {
                    array_splice($query['metrics'], $rem_index, 1);
                }
                
                $goalid = array_get_index($args, 'goalid');
                if ($goalid === 'all') {
                    $goal_stat = 'ga:goalCompletionsAll';
                } else {
                    $goal_stat = 'ga:goal'.$goalid.'Completions';
                }

                $query['metrics'][] = 'ga:sessions';
                $query['metrics'][] = $goal_stat;

                if ($context === 'goal') {
                    $query['segment'][] = 'sessions::condition::'.$goal_stat.'>0';
                } else {
                    $query['segment'][] = 'sessions::condition::'.$goal_stat.'==0';
                }
            break;

            case 'experiment':
                $experimentid = array_get_index($args, 'experimentid');
                $query['segment'][] = 'sessions::condition::ga:experimentId=='.$experimentid;
                $query['metrics'][] = 'ga:sessions';

                // TODO: 
                $view = $this->getView();
                $timezone = new DateTimeZone($view['timezone']);
                $exps = $this->getExperiments();
                $exp  = NULL;

                // Find the experiment we want.
                foreach ($exps as $this_exp) {
                    if ($this_exp['id'] === $experimentid) {
                        $exp = $this_exp;
                        break;
                    }
                }

                // Lock down the filer start/end date times to the hour that it
                // was started/ended (as close as GA lets us do with a single dimension).
                // We currently INCLUDE the hour when it was created
                // so if it started at 09:30 and ended at 09:30, then those times will
                // become 09:00 and 10:00 respectively.
                if (($exp['status'] === 'RUNNING') || ($exp['status'] === 'ENDED')) {
                    $start_date = new DateTime($exp['startTime']);
                    $start_date->setTimezone($timezone);
                    //$query['filter'][] = 'ga:dateHour>='.$start_date->format('YmdH');
                    $query['start_date'] = $start_date->format('Y-m-d');
                }

                if ($exp['status'] === 'ENDED') {
                    $end_date = new DateTime($exp['endTime']);
                    $end_date->setTimezone($timezone);
                    //$query['filter'][] = 'ga:dateHour<='.$end_date->format('YmdH');
                    $query['end_date'] = $end_date->format('Y-m-d');
                }
            break;

            case 'all':
                // Don't filter, we want all the site data.
                $query['metrics'][] = 'ga:sessions';
                $query['sort'][] = '-ga:sessions';
            break;

            default:
                trigger_localised_error('GANA0001', sprintf(translate('Cannot get canned Google Analytics query; invalid context "%s"'), $context), E_USER_WARNING);
                return FALSE;
            break;
        }//end switch context

        switch ($queryid) {
            case 'current':
                // Do nothing - what we have is fine.
            break;
            case 'previous':
                $end_date->sub(new DateInterval($period));
                $start_date->sub(new DateInterval($period));
                $query['end_date']   = $end_date->format('Y-m-d');
                $query['start_date'] = $start_date->format('Y-m-d');
            break;

            case 'by_day':
                $query['dimensions'] = Array(
                    'ga:nthDay',
                    'ga:userType',
                );
                $query['sort'] = $query['dimensions'];
            break;

            case 'by_day_alt':
                $query['dimensions'] = Array(
                    'ga:nthDay',
                );
                $query['sort'] = $query['dimensions'];
                $query['metrics'] = Array(
                    'ga:pageviews',
                    'ga:uniquePageviews',
                    'ga:sessions',
                    'ga:avgTimeOnPage',
                    'ga:entrances',
                    'ga:exits',
                    'ga:bounceRate',
                    'ga:pageLoadTime',
                    'ga:pageLoadSample',
                );
            break;

            case 'campaigns':
                // Remove page load data because the combination of it and
                // ga:users is not compatible with traffic source dimensions
                $rem_index = array_search('ga:pageLoadTime', $query['metrics']);
                if ($rem_index !== FALSE) {
                    array_splice($query['metrics'], $rem_index, 1);
                }
                $rem_index = array_search('ga:pageLoadSample', $query['metrics']);
                if ($rem_index !== FALSE) {
                    array_splice($query['metrics'], $rem_index, 1);
                }
                $query['dimensions'] = Array(
                    'ga:medium',
                    'ga:source',
                );
            break;

            case 'most_viewed_device':
                $query['dimensions'] = Array(
                    'ga:deviceCategory',
                    'ga:hostname',
                    'ga:pagePath'
                );
            break;

            case 'previous_path':
                $query['dimensions'] = Array(
                    'ga:previousPagePath',
                );
                $query['sort'] = Array('-ga:pageviews');
                $query['filter'][] = 'ga:previousPagePath!=(entrance)';
            break;

            case 'device':
                $query['dimensions'] = Array(
                    'ga:deviceCategory',
                );
            break;

            case 'device_daily':
                $query['dimensions'] = Array(
                    'ga:deviceCategory',
                    'ga:nthDay',
                );
            break;

            case 'referral_pages':
                // Remove page load data because the combination of it and
                // ga:users is not compatible with traffic source dimensions
                $rem_index = array_search('ga:pageLoadTime', $query['metrics']);
                if ($rem_index !== FALSE) {
                    array_splice($query['metrics'], $rem_index, 1);
                }
                $rem_index = array_search('ga:pageLoadSample', $query['metrics']);
                if ($rem_index !== FALSE) {
                    array_splice($query['metrics'], $rem_index, 1);
                }
                $query['dimensions'] = Array(
                    'ga:source',
                    'ga:referralPath',
                );
                $query['filter'][] = 'ga:medium==referral';
            break;

            case 'os':
                $query['dimensions'] = Array(
                    'ga:operatingSystem',
                    'ga:operatingSystemVersion',
                    'ga:deviceCategory',
                );
            break;

            case 'browser':
                $query['dimensions'] = Array(
                    'ga:browser',
                    'ga:browserVersion',
                    'ga:deviceCategory',
                );
            break;

            case 'location':
                $query['dimensions'] = Array(
                    'ga:city',
                    'ga:region',
                    'ga:country',
                );
            break;

            case 'language':
                $query['dimensions'] = Array(
                    'ga:language',
                );
            break;

            case 'orgs':
                $query['dimensions'] = Array(
                    'ga:networkLocation',
                    'ga:networkDomain',
                );
            break;

            case 'goal_summary':
                // Just ignore the dimensions for now - we need
                // to go to a separate method for this later.
            break;

            case 'page_path':
                $query['dimensions'] = Array(
                    'ga:pagePath',
                );

                // Replace sessions with unique page views. But then page speed
                // metrics are not a valid combination.
                $remove_metrics = Array(
                    'ga:sessions',
                    'ga:pageLoadTime',
                    'ga:pageLoadSample',
                );

                foreach ($remove_metrics as $remove_metric) {
                    $rem_index = array_search($remove_metric, $query['metrics']);
                    if ($rem_index !== FALSE) {
                        array_splice($query['metrics'], $rem_index, 1);
                    }

                    $rem_index = array_search($remove_metric, $query['sort']);
                    if ($rem_index !== FALSE) {
                        array_splice($query['sort'], $rem_index, 1);
                    }

                    $rem_index = array_search('-'.$remove_metric, $query['sort']);
                    if ($rem_index !== FALSE) {
                        array_splice($query['sort'], $rem_index, 1);
                    }
                }

                $query['metrics'][] = 'ga:uniquePageviews';
                $query['sort'][]    = '-ga:uniquePageviews';
            break;

            case 'page_path_conversions':
                if ($context !== 'goal') {
                    trigger_localised_error(
                        'GANA0003',
                        sprintf(
                            translate('Cannot get canned Google Analytics query; query ID "%s" only makes sense in context "%s"'),
                            'page_path_conversions',
                            'goal'
                        ),
                        E_USER_WARNING
                    );
                    return FALSE;
                }

                $query['dimensions'] = Array(
                    'ga:pagePath',
                );

                // Replace sessions with unique page views. But then page speed
                // metrics are not a valid combination.
                $remove_metrics = Array(
                    'ga:sessions',
                    'ga:pageLoadTime',
                    'ga:pageLoadSample',
                );

                foreach ($remove_metrics as $remove_metric) {
                    $rem_index = array_search($remove_metric, $query['metrics']);
                    if ($rem_index !== FALSE) {
                        array_splice($query['metrics'], $rem_index, 1);
                    }
                }

                $query['metrics'][] = 'ga:uniquePageviews';

                if ($goalid === 'all') {
                    $goal_stat = 'ga:goalCompletionsAll';
                } else {
                    $goal_stat = 'ga:goal'.$goalid.'Completions';
                }
                $query['filter'][]  = $goal_stat.'>0';
            break;

            default:
                trigger_localised_error('GANA0002', sprintf(translate('Cannot get canned Google Analytics query; invalid canned query ID "%s"'), $queryid), E_USER_WARNING);
                return FALSE;
            break;
        }//end switch query ID

        if ($context === 'experiment') {
            $query['dimensions'][] = 'ga:experimentVariant';
        }

        // Unique Pageviews is not compatible with Session Duration.
        if (in_array('ga:uniquePageviews', $query['metrics']) === TRUE) {
            $rem_index = array_search('ga:sessionDuration', $query['metrics']);
            if ($rem_index !== FALSE) {
                array_splice($query['metrics'], $rem_index, 1);
            }
        }

        // Recalculate the period.
        $new_period = $start_date->diff($end_date, TRUE);

        $age = 0;
        if ($use_cache === TRUE) {
            $data = $connector->loadFromFetchCache($cache_key, 0);
            if ($data === NULL) {
                $data = Array(
                    'start-date' => $start_date->format('Y-m-d'),
                    'end-date'   => $end_date->format('Y-m-d'),
                    'period'     => ($new_period->days + 1),
                    'age'        => NULL,
                    'rows'       => NULL,
                );
            } else {
                $age = $connector->getFetchCacheAge($cache_key);
                $data['age'] = $age;
            }//end if no data in cache
        } else {
            switch ($queryid) {
                case 'goal_summary':
                    $data = $this->_getGoalSummaryCannedReport($query);
                break;

                default:
                    $data = $this->generateReport(
                        $query['dimensions'],
                        $query['metrics'],
                        $query['start_date'],
                        $query['end_date'],
                        $query['sort'],
                        $query['filter'],
                        $query['segment'],
                        'nocache'
                    );
                break;
            }//end switch

            if (($data === NULL) || (array_key_exists('errors', $data) === TRUE)) {
                return $data;
            } else {
                $connector->saveToFetchCache($cache_key, $data, $temp_suffix);
                $data['age'] = 0;
            }
        }//end if use cache

        return $data;
    
    }//end getCannedReport()


    /**
     * Created a canned report based on a summary of sessions,
     * goal completions and starts.
     *
     * This is actually multiple queries folded into one, to try
     * and beat the ten dimension limit.
     */
    private function _getGoalSummaryCannedReport($query)
    {
        $view = $this->getView();

        $metrics = Array('ga:sessions');
        $goals   = $this->getGoals();
        foreach ($goals as $goal) {
            $goalid = $goal['id'];
            $metrics[] = 'ga:goal'.$goalid.'Completions';
            $metrics[] = 'ga:goal'.$goalid.'Value';
            if ($goal['type'] === 'URL_DESTINATION') {
                if (empty($goal['urlDestinationDetails']['steps']) === FALSE) {
                    $metrics[] = 'ga:goal'.$goalid.'Starts';
                }
            }
        }
 
        $all_data = Array();
        foreach(array_chunk($metrics, 10) as $chunk) {
            $data = $this->generateReport(
                Array(),
                $chunk,
                $query['start_date'],
                $query['end_date'],
                Array(),
                $query['filter'],
                $query['segment'],
                'nocache'
            );

            if ($data === NULL) {
                return NULL;
            } else if (empty($all_data) === TRUE) {
                $all_data = $data;
            } else {
                $all_data['rows'][0] = array_merge($all_data['rows'][0], $data['rows'][0]);
            } 
        }

        // Add the currency to the row so we can show what the goal
        // value is denominated in
        $all_data['rows'][0]['sq:profileCurrency'] = $view['currency'];

        return $all_data;

    }//end _getGoalSummaryCannedReport()

}//end class

?>
