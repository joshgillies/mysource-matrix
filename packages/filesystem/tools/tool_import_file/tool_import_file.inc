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
* $Id: tool_import_file.inc,v 1.4 2012/12/05 05:43:28 cupreti Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Import_File
*
* Purpose
*
*    Tool for importing a file/folder from under a file bridge
*
* @author Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage file
*/

class Tool_Import_File extends Tool
{


	/**
	* Constructor
	*
	* @param int  $assetid  the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	static function paintTool(&$o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);
			$o->openField(translate('File Bridge Root Node'));

				$root_nodes = Array(
								'physical_file'		=> 'I',
								'physical_folder'	=> 'I',
								'file_bridge'		=> 'I',
							 );
				asset_finder($type_code.'_importing_root_assetid', '', $root_nodes);
				$o->note(translate('If a file is selected as the root node, it will be imported as a new File asset. If a folder or file bridge is selected, then all files and folders underneath will be imported, and not the selected node.'));

			$o->closeField();
			$o->openField(translate('New Parent Node'));

				asset_finder($type_code.'_importing_parent_assetid', '');
			$o->closeField();
			$o->openField(translate('Recursive'));

				check_box($type_code.'_recursive', '1');
				$o->note(translate('If selected, the import tool will import the contents of any folder found underneath the root node'));

			$o->closeField();
			$o->openField(translate('Import "index.html" as a Standard Page'));

				check_box($type_code.'_index_file', '1');
				$o->note(translate('If selected, the file "index.html", if any, will be imported as a Standard Page asset and will replace the folder containing this index file'));

			$o->closeField();
			$o->openField(translate('Allow Virus Scan'));

				combo_box($type_code.'_allow_virus_scan', Array(1 => 'Yes', 0 => 'No'));
				$o->note(translate('If global Virus Checker is enabled, perform immediate virus scan on each imported file. This will slow down import process.'));

			$o->closeField();
		$o->closeSection();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	static function processTool(&$o, $type_code)
	{
		$importr = 0;
		$importp = 0;
		$recursive = FALSE;
		$allow_virus_scan = TRUE;

		if (isset($_REQUEST[$type_code.'_importing_root_assetid']['assetid'])) {
			$importr = $_REQUEST[$type_code.'_importing_root_assetid']['assetid'];
		}

		if (isset($_REQUEST[$type_code.'_importing_parent_assetid']['assetid'])) {
			$importp = $_REQUEST[$type_code.'_importing_parent_assetid']['assetid'];
		}

		if (isset($_REQUEST[$type_code.'_allow_virus_scan'])) {
			$allow_virus_scan = $_REQUEST[$type_code.'_allow_virus_scan'];
		}

		$recursive = (array_get_index($_REQUEST, $type_code.'_recursive', FALSE)) ? TRUE : FALSE;
		$index_file = (array_get_index($_REQUEST, $type_code.'_index_file', FALSE)) ? TRUE : FALSE;

		$sanity_check = TRUE;
		$allowable_root_nodes = Array('physical_file', 'physical_folder', 'file_bridge');
		$root_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($importr);
		if (is_null($root_node) || !in_array(strtolower($root_node->type()), $allowable_root_nodes)) {
			$sanity_check = FALSE;
			if (!is_null($root_node)) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_node, TRUE);
			}//end if
		}//end if

		if (!empty($importr) && !empty($importp) && $sanity_check) {
			$vars = Array(
						'root_assetid' 		=> Array($importr),
						'parent_assetid'	=> $importp,
						'recursive'			=> $recursive,
						'index_file'        => $index_file,
						'allow_virus_scan'	=> $allow_virus_scan,
					);

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_import_file', $vars, '', SQ_PACKAGES_PATH.'/filesystem/hipo_jobs');
			$url = $hh->runQueuedJobs();
			if (!empty($url)) $o->setRedirect($url);
			return TRUE;
		} else {
			// Fail
			if (empty($importr) && $sanity_check) {
				trigger_localised_error('FIL0001', E_USER_ERROR);
			} else {
				trigger_localised_error('FIL0003', E_USER_ERROR);
			}//end if
			return FALSE;
		}//end if

	}//end processTool()


}//end class


?>
