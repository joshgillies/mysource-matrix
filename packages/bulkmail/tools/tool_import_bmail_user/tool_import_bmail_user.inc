<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: tool_import_bmail_user.inc,v 1.1 2005/12/06 04:30:26 rong Exp $
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
* @version $Revision: 1.1 $
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
	* @return string
	* @access public
	*/
	function paintTool(&$o, $type_code)
	{
		ob_start();

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

		$o->openField('', 'commit');
			echo '&nbsp;';
			hidden_field('committed_tool_type_code', $type_code);
			submit_button('tool_commit', SQ_CONF_COMMIT_BUTTON_TEXT);
		$o->closeField();

		$o->closeSection();
		return ob_get_clean();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	function processTool(&$o, $type_code)
	{
		// error: csv file not specified
		$file_info = get_file_upload_info($type_code.'_file_upload');
		if (empty($file_info)) {
			trigger_localised_error('BML0011', E_USER_NOTICE);
			Tool_Import_Bmail_User::paintTool($o, $type_code);
			return;
		}
		// error: user group not specified
		if (!$_REQUEST[$type_code.'_create_in_assetid']['assetid']) {
			trigger_localised_error('BML0012', E_USER_NOTICE);
			Tool_Import_Bmail_User::paintTool($o, $type_code);
			return;
		}

		// set HIPO running_vars
		$vars['file_info'] = $file_info;
		$vars['create_in_assetid'] = $_REQUEST[$type_code.'_create_in_assetid']['assetid'];

		// run HIPO job
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_import_bmail_user', $vars, '', SQ_PACKAGES_PATH.'/bulkmail/hipo_jobs');
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

	}//end processTool()


}//end class


?>
