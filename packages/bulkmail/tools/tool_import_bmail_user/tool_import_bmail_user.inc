<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: tool_import_bmail_user.inc,v 1.4.2.1 2007/01/08 05:23:54 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Import_Bmail_User
*
* Purpose
*
* Tool for importing Bulkmail Users from a CSV file
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.4.2.1 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/

class Tool_Import_Bmail_User extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid  the asset id to be loaded
	*
	*/
	function Tool_Import_Bmail_User($assetid=0)
	{
		$this->Tool($assetid);

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
	function paintTool(&$o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		$o->openField(translate('bm_tool_import_bmail_user_file_upload'));
			file_upload($type_code.'_file_upload');
			$o->note(translate('bm_tool_import_bmail_user_file_upload_note'));
		$o->closeField();

		$o->openField(translate('bm_tool_import_bmail_user_create_in'));
			asset_finder($type_code.'_create_in_assetid', 0, Array('user_group' => 'D'));
			$o->note(translate('bm_tool_import_bmail_user_create_in_note'));
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
	function processTool(&$o, $type_code)
	{
		// error: csv file not specified
		$file_info = get_file_upload_info($type_code.'_file_upload');
		if (empty($file_info)) {
			trigger_localised_error('BML0011', E_USER_NOTICE);
			return FALSE;
		}
		// error: user group not specified
		if (!$_REQUEST[$type_code.'_create_in_assetid']['assetid']) {
			trigger_localised_error('BML0012', E_USER_NOTICE);
			return FALSE;
		}

		// set HIPO running_vars
		$vars['file_info'] = $file_info;
		$vars['create_in_assetid'] = $_REQUEST[$type_code.'_create_in_assetid']['assetid'];

		// run HIPO job
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_import_bmail_user', $vars, '', SQ_PACKAGES_PATH.'/bulkmail/hipo_jobs');
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


}//end class


?>
