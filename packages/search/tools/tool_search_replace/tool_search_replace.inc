<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: tool_search_replace.inc,v 1.2 2005/03/18 05:23:04 dheppell Exp $
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
* @version $Revision: 1.2 $
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
	function Tool_Search_Replace($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Gather required HIPO data
	*
	* @param Object &$o reference to the outputter object
	* @param String $type_code type code of this asset
	* @access public
	*/
	function paintTool(&$o, $type_code)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		ob_start();

		//user input of search and replace strings
		$o->openSection('Search Terms');
			$o->openField('Search String');
				text_box($type_code.'_search_string', '', 40);
			$o->closeField();

			$o->openField('Replace String');
				text_box($type_code.'_replace_string', '', 40);
			$o->closeField();
		$o->closeSection();

		//allow restrictions based on asset type and asset status
		$o->openSection('Search Restrictions');
			$o->openField('Asset Types');
				$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();

				foreach ($asset_types as $type => $info) $asset_types[$type] = $info['name'];
				asort($asset_types, SORT_STRING);

				combo_box($type_code.'_search_types', $asset_types, true, '', 10);
			$o->closeField();

			$o->openField('Asset Status');
				$statuses = get_constant_values('SQ_STATUS');
				$options = Array();
				foreach ($statuses as $constant => $value) $options[$value] = get_status_description($value);
				combo_box($type_code.'_statuses', $options, true, '', 10);
			$o->closeField();
		$o->closeSection();

		//limit the search to certain parts (or all) of the system
		$o->openSection('Search Location');
			$o->openField('Root Node');
				asset_finder($type_code.'_root_assetid', '');
				echo '&nbsp;&nbsp;&nbsp;<b>OR</b>';
			$o->closeField();

			$o->openField('System Wide');
				check_box($type_code.'_root_folder');
				echo 'Search entire system';
			$o->closeField();

			$o->openField('', 'commit');
				echo '&nbsp;';
				hidden_field('committed_tool_type_code', $type_code);
				submit_button('tool_commit', 'Commit');
			$o->closeField();
		$o->closeSection();

		return ob_get_clean();


	}//end paintTool()


	/**
	* Call search replace HIPO
	*
	* Processes the information input by the user and runs the search. Launches
	* a HIPO to perform the replacements with the formatted search information.
	*
	* @param Object &$o reference to the outputter object
	* @param String $type_code type code of this asset
	* @access public
	*/
	function processTool(&$o, $type_code)
	{
		//ensure we have a search string
		if ($_REQUEST[$type_code.'_search_string'] == '') {
			trigger_error('Required search string was blank', E_USER_WARNING);
			Tool_Search_Replace::paintTool($o, $type_code);
			return;
		}

		if (!isset($_REQUEST[$type_code.'_search_types'])) {
			trigger_error('No asset types were selected', E_USER_WARNING);
			Tool_Search_Replace::paintTool($o, $type_code);
			return;
		}

		if (!isset($_REQUEST[$type_code.'_statuses'])) {
			trigger_error('No asset statuses were selected', E_USER_WARNING);
			Tool_Search_Replace::paintTool($o, $type_code);
			return;
		}

		if (!isset($_REQUEST[$type_code.'_root_assetid']) && !isset($_REQUEST[$type_code.'_root_folder'])) {
			trigger_error('No root has been specified', E_USER_WARNING);
			Tool_Search_Replace::paintTool($o, $type_code);
			return;
		}

		$sm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

		//create search types array
		$search_types = array_flip($_REQUEST[$type_code.'_search_types']);
		foreach ($search_types as $index => $value) $search_types[$index] = 1;

		//work out our root node position
		if (isset($_REQUEST[$type_code.'_root_folder'])) {
			$root_assetid = 1;
		} else {
			$root_assetid = $_REQUEST[$type_code.'_root_assetid']['assetid'];
		}

		$words = strtolower($_REQUEST[$type_code.'_search_string']);
		$words = explode(' ', $words);

		$search_details = Array(
								'logic'           => 'AND',
								'root_assetid'    => $root_assetid,
								'sort_by'         => '',
								'reverse_sort'    => '0',
								'search_types'    => $search_types,
								'search_statuses' => $_REQUEST[$type_code.'_statuses'],
								'include_all'     => Array (
														   'search_word' => Array (
																				  'words'        => $words,
																				  'search_logic' => 'AND',
																				  ),
														   ),
								);

		$search_results = $sm->processSearch($search_details, true);

		//no matches were found, trigger error and paint getInterface
		if (!$search_results) {
			trigger_error('No Matches for '.$_REQUEST[$type_code.'_search_string'].' could be found', E_USER_WARNING);
			Tool_Search_Replace::paintTool($o, $type_code);
			return;
		}

		$formatted_results = Tool_Search_Replace::getFormattedResults($search_results, $_REQUEST[$type_code.'_search_string']);

		//after formatting the results, there were no real replaceable matches
		if (empty($formatted_results)) {
			trigger_error('No Matches for '.$_REQUEST[$type_code.'_search_string'].' could be found', E_USER_WARNING);
			Tool_Search_Replace::paintTool($o, $type_code);
			return;
		}

		$vars = Array(
					  'search_data'    => $formatted_results,
					  'search_string'  => $_REQUEST[$type_code.'_search_string'],
					  'replace_string' => $_REQUEST[$type_code.'_replace_string'],
					  );

		//redirect data to the HIPO
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_search_replace', $vars, '', SQ_PACKAGES_PATH.'/search/tools/tool_search_replace');
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

	}//end processTool()


	/**
	* Provide generic format of search results data
	*
	* Results array is in the format returned by search manager
	* when a detailed searched is made
	*
	* @access public
	*/
	function getFormattedResults($results, $search_string)
	{
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$formatted_results = Array();

		//transform the array into a better working format
		foreach ($results as $result) {
			$searched_results[$result['assetid']][] = $result['source'];
		}

		//populate the results array as required
		//note we dont include fields that dont have matches
		foreach ($searched_results as $assetid => $content) {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			foreach ($content as $content_type) {
				switch ($content_type['source']) {

					//asset contents containing search string
					case preg_match('/contents/', $content_type) > 0:
						$asset_contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($assetid);

						foreach ($asset_contents as $contents_id => $contents_value) {
							if (preg_match("/$search_string/i", $contents_value)) {
								$formatted_results[$assetid]['contents'][$contents_id] = $contents_value;
							}
						}
						break;

					//attributes containing search string
					//attributes come in 2 formats, __attribtue__ and attr:assetname:attribute$
					case preg_match('/__(.+)__/', $content_type, $matches) > 0:
					case preg_match("/attr:.+:(.+)\$/", $content_type, $matches) > 0:
						if (isset($asset->vars[$matches[1]])) {
							$attribute = $asset->attr($matches[1]);

							if (preg_match("/$search_string/i", $attribute)) {
								$formatted_results[$assetid]['attributes'][$matches[1]] = $attribute;
							}
						}
					break;

					//metadata containing search string
					case preg_match("/metadata/", $content_type) > 0:
						//we've already set metadata for this asset
						if (isset($formatted_results[$assetid]['metadata'])) break;

						$metadata = $mm->getMetadata($assetid);

						foreach ($metadata as $fieldid => $data) {
							if (preg_match("/$search_string/i", $data['value'])) {
								$formatted_results[$assetid]['metadata'][$fieldid] = $data;
							}
						}

						//if no metadata was set, then we were looking for the schema
						if (!isset($formatted_results[$assetid]['metadata'])) {
							$schemas = $mm->getSchemas($assetid, true);

							foreach ($schemas as  $schemaid) {
								$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
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
