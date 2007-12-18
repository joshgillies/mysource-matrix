<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: backend_search.inc,v 1.7 2007/12/18 23:47:04 rong Exp $
*
*/


/**
* Backend_Search
*
* Purpose
*    This abstracts out the enhanced backend searching of the
*    from the package.xml and asset.xml files
*
* @static
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/
class Backend_Search
{


	/**
	* Is backend search available?
	*
	* For backend search to be available, the search manager must be installed.
	*
	* @static
	* @return boolean
	* @access public
	*/
	public static function isAvailable()
	{
		return $GLOBALS['SQ_SYSTEM']->am->installed('search_manager');

	}//end isAvailable()


	/**
	* Process a simple search for the backend interface
	*
	* The $restrict_statuses parameter, if specified, should be an array of
	* statuses, rather than a bitmask,
	*     eg. Array(SQ_STATUS_UNDER_CONSTRUCTION, SQ_STATUS_LIVE)
	*
	* The $restrict_asset_types parameter, if specified, should be specified as
	* an array of (asset type => inherit flag)
	*     eg. Array('file' => TRUE, 'folder' => FALSE)
	*
	* @param string	$search_terms			The search term(s) to search upon
	* @param array	$restrict_statuses		If not empty, restrict statuses to these
	*										ones, otherwise search all statuses
	* @param array	$restrict_asset_types	If not empty, restrict types searched for
	*										to these ones, otherwise search all
	*										asset types
	*
	* @static
	* @return array
	* @access public
	*/
	public static function processSearch($search_terms, $restrict_statuses=Array(), $restrict_asset_types=Array())
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		if (Backend_Search::isAvailable()) {
			$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

			// Are we restricting statuses? If not, use all of them
			if (empty($restrict_statuses)) {
				$restrict_statuses = get_constant_values('SQ_STATUS_');
			}

			$search_info = Array(
							'fields'	=> Array(
											'backend'	=> Array(
															'words'			=> $search_terms,
															'word_logic'	=> 'AND',
															'data_sources'	=> Array(
																				0 => Array('type' => 'include_all'),
																			   ),
														   ),
										   ),
							'statuses'	=> $restrict_statuses,
						   );

			// Are we restricting by asset types? If so, add them as a condition
			if (!empty($restrict_asset_types)) {
				$search_info['asset_types'] = $restrict_asset_types;
			}

			// Do the search
			$results = $sm->processSearch($search_info, TRUE);

			if (!empty($results)) {
				$result_limit = $GLOBALS['SQ_SYSTEM']->getUserPrefs('search_manager', 'SQ_SEARCH_BACKEND_RESULT_LIMIT');
				$results = $sm->getCondensedResults($results);
				if ($result_limit > 0) {
					$results = array_slice($results, 0, $result_limit, TRUE);
				}
				$results = $sm->getFormattedResults($results, $search_terms);

			}
		} else {
			$results = Array();
		}

		return $results;

	}//end processSearch()


}//end class

?>
