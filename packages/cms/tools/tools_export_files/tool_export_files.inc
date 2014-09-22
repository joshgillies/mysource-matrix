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
* $Id: tool_export_files.inc,v 1.3 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Export_Files
*
* Purpose
*
* Tool for exporting files
*
* @author Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/

class Tool_Export_Files extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid  the asset id to be loaded
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
		$filetypes = Array(
						'folder'	=> 'D',
					 );

		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$itm = $am->getSystemAsset('import_tools_manager');

		$save_name = $itm->attr('cms_file_export_name');
		$save_location = $itm->attr('cms_file_export_dir');
		if(!empty($save_location) && strpos($save_location, '/') !== 0) {
		    $save_location = SQ_SYSTEM_ROOT.'/'.$save_location;
		}

		$o->openSection($tool_info['name']);

			$o->openField(translate('Root Node'));

				multiple_asset_finder($type_code.'_root_node', Array(), $filetypes);
				$o->note(translate('Export all the files under this root node'));

			$o->closeField();

			$o->openField(translate('Save As'));

				text_box($type_code.'_save_name', $save_name, 30);
				$o->note(translate('Name to save the exported files as'));

			$o->closeField();

			$o->openField(translate('Save Location'));

				text_box($type_code.'_save_location', $save_location, 30);
				$o->note(translate('Location to save the exported files'));

			$o->closeField();

			$o->openField(translate('Keep Structure'));

				check_box($type_code.'_keep', 1, TRUE);
				$o->note(translate('When exporting, keep the structure underneath the root node'));

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
		// error: root node not specified
		if (!isset($_REQUEST[$type_code.'_root_node']) || empty($_REQUEST[$type_code.'_root_node'])) {
			trigger_localised_error('CMS0100', E_USER_NOTICE);
			return FALSE;
		}//end if

		// error: save name not specified
		if (!isset($_REQUEST[$type_code.'_save_name']) || empty($_REQUEST[$type_code.'_save_name'])) {
			trigger_localised_error('CMS0101', E_USER_NOTICE);
			return FALSE;
		}//end if

		// error: save location not specified
		if (!isset($_REQUEST[$type_code.'_save_location']) || empty($_REQUEST[$type_code.'_save_location'])) {
			trigger_localised_error('CMS0102', E_USER_NOTICE);
			return FALSE;
		}//end if

		// error: save location not directory and not writable
		if (!is_dir($_REQUEST[$type_code.'_save_location']) || !is_writable($_REQUEST[$type_code.'_save_location'])) {
			trigger_localised_error('CMS0103', E_USER_NOTICE);
			return FALSE;
		}//end if

		// set HIPO running_vars
		$vars['root_node'] = $_REQUEST[$type_code.'_root_node'];
		$vars['save_name'] = $_REQUEST[$type_code.'_save_name'];
		$vars['save_location'] = $_REQUEST[$type_code.'_save_location'];
		$vars['keep_structure'] = (isset($_REQUEST[$type_code.'_keep']) && $_REQUEST[$type_code.'_keep']) ? TRUE : FALSE;

		// run HIPO job
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_export_files', $vars, '', SQ_PACKAGES_PATH.'/cms/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

	}//end processTool()


}//end class


?>
