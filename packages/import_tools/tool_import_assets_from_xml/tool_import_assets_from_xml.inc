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
* $Id: tool_import_assets_from_xml.inc,v 1.2 2010/09/22 05:54:05 akarelia Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Tool_Import_Assets_From_Xml
*
* Purpose
*
*    Tool for importing assets into Matrix
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/

class Tool_Import_Assets_From_Xml extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Import_Assets_From_Xml($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return string
	* @access public
	*/
	public static function paintTool(&$o, $type_code)
	{

		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		$o->openField(translate('tool_import_assets_from_xml_file_upload'));
			file_upload($type_code.'_file_upload');
			$o->note(translate('tool_import_assets_from_xml_file_upload_note', sq_web_path('data').'/asset_types/tool_import_assets_from_xml/files/example.xml'));
		$o->closeField();

		$o->openField(translate('tool_import_assets_from_xml_create_under'));
			asset_finder($type_code.'_create_under_assetid', 0, Array('asset' => 'D'));
			$o->note(translate('tool_import_assets_from_xml_create_under_note'));
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
	public static function processTool(&$o, $type_code)
	{
		// error: xml file not specified
		$file_info = get_file_upload_info($type_code.'_file_upload');
		if (empty($file_info)) {
			trigger_localised_error('IMP0002', E_USER_NOTICE);
			return FALSE;
		} else {
			if(substr($file_info['name'], -4) != '.xml') {
				//not an XML file :-o Abort Abort !
				trigger_error('File Provided not a XML file', E_USER_ERROR);
				return FALSE;
			}

			$filename = str_replace('.xml', '', $file_info['name']);
			//upload successful move the file to data directory
			$i = 0;

			$temp_filename = SQ_DATA_PATH.'/public/'.$filename.'_'.$i.'.xml';
			while(!move_file($file_info['tmp_name'], $temp_filename))
			{
				if($i == 10 ){
					//tried too many time? quit now
					trigger_error('Could not move uploaded file from tmp directory to data dir');
					return FALSE;
				}
				// looks we couldnt move the file coz it prolly already exists
				// try changinghte file name
				$i++;
				$temp_filename = SQ_DATA_PATH.'/public/'.$filename.'_'.$i.'.xml';
			}
			$om = umask(0000);
			chmod($temp_filename, 0664);
			umask($om);
		}

		$root_node_id 	= $_POST[$type_code.'_create_under_assetid']['assetid'];
		// if value of root node is 0 then we are good. It just means we are not selecting root node
		// but if we have a value, then make sure it is a proper asset
		if($root_node_id != 0) {
			$root_node 		= $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node_id);
			if (is_null($root_node)) {
				trigger_localised_error('IMP0003', E_USER_NOTICE);
				return FALSE;
			}
		}

		// set HIPO running_vars
		$vars['file_info'] = $temp_filename;
		$vars['create_under_assetid'] = $root_node_id;
		$vars['delete_after_import'] = TRUE;

		// run HIPO job
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_import_assets_from_xml', $vars, '', SQ_PACKAGES_PATH.'/import_tools/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

		return TRUE;


	}//end processTool()


}//end class


?>
