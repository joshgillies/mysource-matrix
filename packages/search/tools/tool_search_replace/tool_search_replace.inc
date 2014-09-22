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
* $Id: tool_search_replace.inc,v 1.31 2012/08/30 00:59:16 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Search_Replace
*
* Purpose
* Provides a tool interface for search and replace operations. Anything that's
* indexed by the search manager can be modified, which includes text attributes,
* metadata and asset content
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @version $Revision: 1.31 $
* @package MySource_Matrix_Packages
* @subpackage search
*/

class Tool_Search_Replace extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Gather required HIPO data
	*
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this asset
	*
	* @return void
	* @access public
	*/
	static function paintTool(&$o, $type_code)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		// user input of search and replace strings
		$o->openSection(translate('Search Terms'));

			$o->openField(translate('Search String'));

				text_box($type_code.'_search_string', '', 40);
			$o->closeField();

			$o->openField(translate('Replace String'));

				text_box($type_code.'_replace_string', '', 40);
			$o->closeField();
			$o->openField('');
				check_box($type_code.'_match_case');
				label(translate('Match Case of Target String'), $type_code.'_match_case');

			$o->closeField();
		$o->closeSection();

		// allow restrictions based on asset type and asset status
		$o->openSection(translate('Search Restrictions'));

			$o->openField(translate('Asset Types'));

				asset_type_chooser($type_code.'_search_types', TRUE, Array());
			$o->closeField();

			$o->openField(translate('Asset Status'));

				$statuses = get_constant_values('SQ_STATUS');
				$options = Array();
				foreach ($statuses as $constant => $value) {
					$options[$value] = get_status_description($value);
				}
				combo_box($type_code.'_statuses', $options, TRUE, '', 10);
			$o->closeField();
		$o->closeSection();

		// limit the search to certain parts (or all) of the system
		$o->openSection(translate('Search Location'));

			$o->openField(translate('Root Node'));

				asset_finder($type_code.'_root_assetid', '');
				echo '&nbsp;&nbsp;&nbsp;<b>'.strtoupper(translate('Or')).'</b>';

			$o->closeField();

			$o->openField(translate('System Wide'));

				check_box($type_code.'_root_folder');
				label(translate('Search entire system'), $type_code.'_root_folder');

			$o->closeField();

		$o->closeSection();

	}//end paintTool()


	/**
	* Call search replace HIPO
	*
	* Processes the information input by the user and runs the search. Launches
	* a HIPO to perform the replacements with the formatted search information.
	*
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this asset
	*
	* @return boolean
	* @access public
	*/
	static function processTool(&$o, $type_code)
	{
		// ensure we have a search string
		if ($_REQUEST[$type_code.'_search_string'] == '') {
			trigger_localised_error('SCH0002', E_USER_NOTICE);
			return FALSE;
		}

		if (!isset($_REQUEST[$type_code.'_search_types'])) {
			trigger_localised_error('SCH0003', E_USER_NOTICE);
			return FALSE;
		}

		if (!isset($_REQUEST[$type_code.'_statuses'])) {
			trigger_localised_error('SCH0004', E_USER_NOTICE);
			return FALSE;
		}

		if (empty($_REQUEST[$type_code.'_root_assetid']['assetid']) && !isset($_REQUEST[$type_code.'_root_folder'])) {
			trigger_localised_error('SCH0005', E_USER_NOTICE);
			return FALSE;
		}

		$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

		if (strlen($_REQUEST[$type_code.'_search_string']) < $sm->attr('min_word_length')) {
			trigger_localised_error('SCH0026', E_USER_NOTICE, $_REQUEST[$type_code.'_search_string'], $sm->attr('min_word_length'));
			return FALSE;
		}

		// create search types array
		$search_types = Array();
		foreach ($_REQUEST[$type_code.'_search_types'] as $type) {
			if (!empty($type)) $search_types[$type] = 1;
		}

		// work out our root node position
		if (isset($_REQUEST[$type_code.'_root_folder'])) {
			$root_assetid = Array();
		} else {
			$root_assetid = Array($_REQUEST[$type_code.'_root_assetid']['assetid']);
		}

		$words = strtolower($_REQUEST[$type_code.'_search_string']);
		$words = explode(' ', $words);

		$search_details = Array(
							'roots'			=> $root_assetid,
							'asset_types'	=> $search_types,
							'statuses'		=> $_REQUEST[$type_code.'_statuses'],
							'fields'		=> Array(
												'search_word'	=> Array(
																	'words'			=> $words,
																	'word_logic'	=> 'AND',
																	'data_sources'	=> Array(
																						Array('type' => 'include_all'),
																					   ),
																   ),
											   ),
							'field_logic'	=> 'AND',
						  );

		$search_results = $sm->processSearch($search_details, TRUE);

		// no matches were found, trigger error and paint getInterface
		if (!$search_results) {
			trigger_localised_error('SCH0006', E_USER_NOTICE, $_REQUEST[$type_code.'_search_string']);
			return FALSE;
		}

		$formatted_results = Tool_Search_Replace::getFormattedResults($search_results, $_REQUEST[$type_code.'_search_string']);

		// after formatting the results, there were no real replaceable matches
		if (empty($formatted_results)) {
			trigger_localised_error('SCH0006', E_USER_NOTICE, $_REQUEST[$type_code.'_search_string']);
			return FALSE;
		}

		$vars = Array(
					'root_assetid'		=> $root_assetid,
					'search_data'		=> $formatted_results,
					'search_string'		=> $_REQUEST[$type_code.'_search_string'],
					'replace_string'	=> $_REQUEST[$type_code.'_replace_string'],
					'match_case'		=> isset($_REQUEST[$type_code.'_match_case']),
					'contextid'			=> $GLOBALS['SQ_SYSTEM']->getContextId(),
				);

		// redirect data to the HIPO
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_search_replace', $vars, '', SQ_PACKAGES_PATH.'/search/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


	/**
	* Provide generic format of search results data
	*
	* Results array is in the format returned by search manager
	* when a detailed searched is made
	*
	* @param array	$results		Search Results
	* @param string	$search_string	String searched for
	*
	* @return array
	* @access public
	*/
	function getFormattedResults($results, $search_string)
	{
		$search_string = preg_replace('/([\.\\\!\+\*\?\[\]\^\$\(\)\=\!\<\>\|\:\/])/i', '\\\${1}',$search_string);

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$formatted_results = Array();

		// transform the array into a better working format
		foreach ($results as $result) {
			$searched_results[$result['assetid']][] = $result['source'];
		}

		// populate the results array as required
		// note we dont include fields that dont have matches
		foreach ($searched_results as $assetid => $content) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			foreach ($content as $content_type) {
				switch ($content_type) {

					// asset contents containing search string
					case preg_match('/contents/', $content_type) > 0:
						$asset_contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($assetid);
						if ($asset_contents === FALSE) break;
						foreach ($asset_contents as $contents_id => $contents_value) {
							if (preg_match('/'.htmlentities($search_string).'/i', $contents_value) || preg_match('/'.($search_string).'/i', $contents_value)) {
								$formatted_results[$assetid]['contents'][$contents_id] = $contents_value;
							}
						}
					break;

					// attributes containing search string
					// attributes come in 2 formats, __attribute__ and attr:attrname$
					case preg_match('/__(.+)__/', $content_type, $matches) > 0:
					case preg_match('/attr:(.+)$/', $content_type, $matches) > 0:
						if (isset($asset->vars[$matches[1]])) {
							$attribute = $asset->attr($matches[1]);

							if (preg_match("/$search_string/i", $attribute)) {
								$formatted_results[$assetid]['attributes'][$matches[1]] = $attribute;
							}
						}
					break;

					// metadata containing search string
					case preg_match('/metadata/', $content_type) > 0:
						// we've already set metadata for this asset
						if (isset($formatted_results[$assetid]['metadata'])) {
							break;
						}

						$metadata = $mm->getMetadata($assetid);

						foreach ($metadata as $fieldid => $data) {
							if (preg_match("/$search_string/i", $data[0]['value'])) {
								$formatted_results[$assetid]['metadata'][$fieldid] = $data[0];
							}
						}

						// if no metadata was set, then we were looking for the schema
						if (!isset($formatted_results[$assetid]['metadata'])) {
							$schemas = $mm->getSchemas($assetid, TRUE);

							foreach ($schemas as  $schemaid) {
								$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
								$schema_data = $mm->getSchemaDefaultValues($schema->id);

								foreach ($schema_data as $fieldid => $data) {
									if (preg_match("/$search_string/i", $data['value'])) {
										$formatted_results[$schema->id]['schema'][$fieldid] = $data;
									}
								}

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($schema);
							}
						}
					break;

					default:
					break;

				}//end switch content type
			}//end foreach $assetid

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

		}//end foreach $results

		return $formatted_results;

	}//end getFormattedResults()


}//end class


?>
