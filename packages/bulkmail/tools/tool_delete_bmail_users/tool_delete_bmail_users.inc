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
* $Id: tool_delete_bmail_users.inc,v 1.3 2008/11/24 22:28:21 mbrydon Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Delete Bulkmail Users Tool
*
* Purpose
* 	A tool to delete Bulkmail User assets that have email addresses
*	matching those in the supplied text file
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/

class Tool_Delete_Bmail_Users extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid  the asset id to be loaded
	*
	*/
	function Tool_Delete_Bmail_Users($assetid=0)
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

		$o->openField(translate('bm_tool_delete_bmail_users_file_upload'));
			file_upload($type_code.'_file_upload');
			$o->note(translate('bm_tool_delete_bmail_users_file_upload_note'));
		$o->closeField();

		$o->openField(translate('bm_tool_delete_bmail_users_confirm'));
			combo_box($type_code.'_confirm', Array(0=>translate('no'), 1=>translate('yes')));
			$o->note(translate('bm_tool_delete_bmail_users_confirm_note'));
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
		if ((!isset($_POST[$type_code.'_confirm'])) || ($_POST[$type_code.'_confirm'] == 0)) {
			return FALSE;
		}

		// set HIPO running_vars
		$vars['file_info'] = $file_info;

		// run HIPO job
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_delete_bmail_users', $vars, '', SQ_PACKAGES_PATH.'/bulkmail/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


}//end class


?>
