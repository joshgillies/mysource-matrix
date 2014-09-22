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
* $Id: tool_import_bmail_user.inc,v 1.11 2012/08/30 00:57:13 ewang Exp $
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
* @version $Revision: 1.11 $
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

		$o->openField(translate('Import CSV File'));

			file_upload($type_code.'_file_upload');
			$o->note(translate('The format for each line must be: "Bulkmail User name, Bulkmail User email address"'));

		$o->closeField();

		$o->openField(translate('Column Headings in CSV File?'));

			check_box($type_code.'_csv_header');
			$o->note(translate('If the first row of your CSV file contains column headings, select this field to ensure Matrix correctly imports the CSV file.'));

		$o->closeField();

		$o->openField(translate('Create Users in'));

			asset_finder($type_code.'_create_in_assetid', 0, Array('user_group' => 'D'));
			$o->note(translate('Select the User Group to create Bulkmail Users in'));

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

		$vars['csv_header'] = 0;
		if (isset($_REQUEST[$type_code.'_csv_header'])) {
			$vars['csv_header'] = 1;
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
