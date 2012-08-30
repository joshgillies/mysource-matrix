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
* $Id: tool_delete_bmail_users.inc,v 1.8 2012/08/30 00:57:13 ewang Exp $
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
* @version $Revision: 1.8 $
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

		$o->openField(translate('bm_tool_delete_bmail_users_root_node'));
			multiple_asset_finder($type_code.'_root_node');
			$o->note(translate('bm_tool_delete_bmail_users_root_node_note'));
		$o->closeField();

		$o->openField(translate('bm_tool_delete_bmail_users_confirm'));
			combo_box($type_code.'_confirm', Array(0=>translate('no'), 1=>translate('yes')));
			$o->note(translate('bm_tool_delete_bmail_users_confirm_note'));
		$o->closeField();

		$o->openField(translate('bm_tool_delete_bmail_users_link_node'));
			asset_finder($type_code.'_link_node');
		  	$o->note(translate('bm_tool_delete_bmail_users_link_node_note'));
		$o->closeField();

		$link_type_list = get_link_type_names();

		$o->openField(translate('bm_tool_delete_bmail_users_link_type'));
			combo_box($type_code.'_link_type', $link_type_list, FALSE);
		  	$o->note(translate('bm_tool_delete_bmail_users_link_type_note'));
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

		$selected_root_nodes = array_get_index($_POST, $type_code.'_root_node', Array());
		$root_nodes = Array();
		if (!empty($selected_root_nodes)) {
			foreach ($selected_root_nodes as $root_node) {
				$assetid = array_get_index($root_node, 'assetid', 0);
				if (!empty($assetid)) {
					$root_nodes[] = $assetid;
				}//end if
			}//end foreach
		}//end if

		$link_node = array_get_index($_POST, $type_code.'_link_node');
		$link_type = array_get_index($_POST, $type_code.'_link_type', Array());

		// set HIPO running_vars
		$vars = Array(
					'file_info'		=> $file_info,
					'root_nodes'	=> $root_nodes,
					'link_asset' 	=> Array(
										'assetid' 	=> $link_node['assetid'],
										'link_type' => $link_type,
									),
				);

		// OK to continue?
		if ((!isset($_POST[$type_code.'_confirm'])) || ($_POST[$type_code.'_confirm'] == 0)) {
			return FALSE;
		}

		// run HIPO job
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_delete_bmail_users', $vars, '', SQ_PACKAGES_PATH.'/bulkmail/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


}//end class


?>
