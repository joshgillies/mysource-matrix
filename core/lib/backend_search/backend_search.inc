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
* $Id: backend_search.inc,v 1.9 2012/08/30 01:09:21 ewang Exp $
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
* @version $Revision: 1.9 $
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
	* The result limit defaults to the search manager preference if unspecified
	* or NULL. If zero, all (remaining) results will be returned.
	*
	* @param string		$search_terms			The search term(s) to search upon
	* @param array		$restrict_statuses		If not empty, restrict statuses to these
	*											ones, otherwise search all statuses
	* @param array		$restrict_asset_types	If not empty, restrict types searched for
	*											to these ones, otherwise search all
	*											asset types
	* @param integer	$result_limit			The number of results to show per page.
	* @param integer	$offset                 Start from this offset.
	*
	* @static
	* @return array
	* @access public
	*/
	public static function processSearch($search_terms, $search_roots=NULL, $restrict_statuses=Array(), $restrict_asset_types=Array(), $result_limit=NULL, $offset=0)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if (Backend_Search::isAvailable()) {
		
			// If the result limit is not specified or less than 0, default to the
			// search manager preference, otherwise truncate to integer.
			if (($result_limit === NULL) || ($result_limit < 0)) {
				$result_limit = $GLOBALS['SQ_SYSTEM']->getUserPrefs('search_manager', 'SQ_SEARCH_BACKEND_RESULT_LIMIT');
			} else {
				$result_limit = (int) $result_limit;
			}

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
			
			// Have we set search roots?
			if (!empty($search_roots)) {
			    $search_info['roots'] = $search_roots;
			}

			// Are we restricting by asset types? If so, add them as a condition
			if (!empty($restrict_asset_types)) {
				$search_info['asset_types'] = $restrict_asset_types;
			}

			// Do the search
			$results = $sm->processSearch($search_info, TRUE);

			if (!empty($results)) {
				$results = $sm->getCondensedResults($results);
				if ($result_limit > 0) {
					$results = array_slice($results, $offset, $result_limit, TRUE);
				}
				$results = $sm->getFormattedResults($results, $search_terms, FALSE);

			}
		} else {
			$results = Array();
		}

		return $results;

	}//end processSearch()


}//end class

?>
