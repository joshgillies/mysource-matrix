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
* $Id: tool_import_bmail_user.inc,v 1.9 2008/11/24 22:28:21 mbrydon Exp $
*
*/

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
* @version $Revision: 1.9 $
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
	public static function paintTool(&$o, $type_code)
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

		$o->openField('');
			check_box($type_code.'_new_link'); echo 'Link user with an existing email address?';
			$o->note('The "email" attribute of a Bulkmail User is unique, so that the same user will not get multiple copies of the same mail out.<br />If an existing address is specified in the CSV file, the Bulkmail User will also be linked under the "Create Users in" folder, if this option is enabled.');
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
		$vars['new_link'] = (isset($_REQUEST[$type_code.'_new_link'])) ? $_REQUEST[$type_code.'_new_link'] : FALSE;

		// run HIPO job
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_import_bmail_user', $vars, '', SQ_PACKAGES_PATH.'/bulkmail/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


}//end class


?>
