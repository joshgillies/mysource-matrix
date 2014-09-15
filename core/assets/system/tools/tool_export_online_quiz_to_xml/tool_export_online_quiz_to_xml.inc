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
* $Id: tool_export_online_quiz_to_xml.inc,v 1.3 2012/08/30 01:09:12 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Tool_Export_Online_Quiz_To_XML
*
* Purpose
*
*    Export Online Quiz Assets to XML File for utilisation in SCORM development.
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Tool_Export_Online_Quiz_To_XML extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Export_Online_Quiz_To_XML($assetid=0)
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

		// do a lock check, only allows one user at a time
		$import_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');
		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($import_mgr->id, 'attributes', TRUE, TRUE);
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$userHasLockId = '';
		$hasLock = TRUE;

		foreach ($locks as $lock) {
		    if (empty($lock)) continue;
		    if ($lock['lock_type'] == 'attributes' && $lock['userid'] == $userid) {
				$hasLock = TRUE;
		    } else if ($lock['lock_type'] == 'attributes' ) {
				$hasLock = FALSE;
				$userHasLockId = $lock['userid'];
		    }
		}

    	if (!$hasLock) {
			$o->openRaw();
			echo (sprintf(translate('tool_export_online_quiz_to_xml_can_not_acquire_lock'), $userHasLockId));
			$o->closeRaw();
			return FALSE;
		}

		$show_download = FALSE;
		if (isset($_REQUEST['show_download'])) {
			$show_download = TRUE;
		} else if (isset($_REQUEST['download'])) {

			$export_dir = SQ_DATA_PATH.'/private/export_assets';
			if (!is_dir($export_dir)) {
				if (!create_directory($export_dir)) return FALSE;
			}

			if (!file_exists($export_dir.'/export.tgz')){
				$command = "cd $export_dir ; tar -czf export.tgz * --remove-files";
				exec($command, $output, $return);

				// exec populate $return with 0 if successful. If not successful, $return will be an exit status code and
				// we don't have a usable compressed file for the user to download
				if ($return !== 0) {
					trigger_localised_error('EXP0001', E_USER_ERROR, $return);
					return;
				}
			}

			// if we don't have a valid filesize, there isn't much point going fruther as the user will end up downloading an unusable file
			$filesize = filesize($export_dir.'/export.tgz');
			if (!$filesize) {
				trigger_localised_error('EXP0002', E_USER_ERROR);
				return;
			}

			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			$ext = 'tgz';
			$type	= (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];

			// We are about to get our download so let go of the lock
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($import_mgr->id, 'attributes');

			// Fix for IE caching
			header("Pragma: public");
			header("Expires: 0");
			header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
			header("Cache-Control: private", FALSE);

			header('Content-Type: '.$type);
			header('Content-Disposition: attachment; filename="export.tgz";');
			header('Content-Length: '.$filesize);

			readfile($export_dir.'/export.tgz');
			exit;
		}

		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		$o->openField(translate('tool_export_online_quiz_to_xml_root_node'));
			asset_finder($type_code.'_root_node');
			$o->note(translate('tool_export_online_quiz_to_xml_root_node_note'));
		$o->closeField();

		$o->openField(translate('tool_export_online_quiz_to_xml_type_codes'));
			$available_types = Array (
									'page_standard' => 'Standard Page',
									'page_online_quiz' => 'Online Quiz'
									);
			combo_box($type_code.'_type_codes', $available_types, TRUE);
			$o->note(translate('tool_export_online_quiz_to_xml_type_codes_note'));
		$o->closeField();

		if ($show_download) {
			$o->openField(translate('download'));
			$backend_url = $GLOBALS['SQ_SYSTEM']->backend->getBackendUrl();
			$download_url = $backend_url.'&backend_section=tools&tool_type_code='.$type_code.'&download=1';
			echo '<a href="'.$download_url.'" >'.translate('download').'</a>';
			$o->closeField();
		}

		$o->closeSection();

		Tool_Export_Online_Quiz_To_XML::paintStandardPageSettings($o, $type_code);

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

		$import_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');
		if(!$GLOBALS['SQ_SYSTEM']->am->acquireLock($import_mgr->id, 'attributes')) {
			trigger_localised_error('EXP0003', E_USER_ERROR);
			return FALSE;
		}

		// make export temp directory
		$export_dir = SQ_DATA_PATH.'/private/export_assets';
		if (!is_dir($export_dir)) {
			if (!create_directory($export_dir)) return FALSE;
		}

		// clean up before export
		if (!clear_directory($export_dir)) return FALSE;

		$root_node = (!empty($_POST[$type_code.'_root_node']['assetid'])) ? $_POST[$type_code.'_root_node']['assetid'] : 0;
		$type_codes = array_get_index($_POST, $type_code.'_type_codes', Array());

		if (empty($root_node)) {
			trigger_error ('There is no root node specified', E_USER_NOTICE);
			return FALSE;
		}

		if (empty($type_codes)) {
			trigger_error ('There is no type codes specified', E_USER_NOTICE);
			return FALSE;
		}

		// set HIPO running_vars
		$vars['export_dir'] = $export_dir;
		$vars['root_node'] = $root_node;
		$vars['type_codes'] = $type_codes;
		$vars['export_settings']['page_standard'] = Tool_Export_Online_Quiz_To_XML::processStandardPageSettings($o, $type_code);

		// run HIPO job
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_export_online_quiz_to_xml', $vars, '');
		$backend_url = $GLOBALS['SQ_SYSTEM']->backend->getBackendUrl();
		// run the hipo job and redirect back to the tool and show download link
		$url = $hh->runQueuedJobs($backend_url.'&backend_section=tools&tool_type_code='.$type_code.'&show_download=1');

		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


	/**
	* Paint the interface for the asset specific export settings of Standard Page
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	public static function paintStandardPageSettings(&$o, $type_code)
	{
		$o->openSection(translate('tool_export_online_quiz_to_xml_standard_page_settings'));

		$o->openField(translate('tool_export_online_quiz_to_xml_paint_method'));
		radio_button($type_code.'_paint_method', 'raw', TRUE);
		label('Raw (no Paint Layout)', $type_code.'_page_standard_paint_method_raw');
		echo '<br / >';
		radio_button($type_code.'_paint_method', 'default_layout');
		label('Using default Paint Layout (based on lookup rules)', $type_code.'_page_standard_paint_method_default_layout');
		$o->closeField();

		$o->openField(translate('tool_export_online_quiz_to_xml_start_tag'));
		text_box($type_code.'_start_tag', '<!-- START TAG -->');
		$o->closeField();

		$o->openField(translate('tool_export_online_quiz_to_xml_end_tag'));
		text_box($type_code.'_end_tag', '<!-- END TAG -->');
		$o->closeField();

		$o->sectionNote(translate('tool_export_online_quiz_to_xml_standard_page_settings_note'));

		$o->closeSection();

	}//end paintStandardPageSettings()


	/**
	* Processes the user input for the asset specific export settings of Standard Page
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return string
	* @access public
	*/
	public static function processStandardPageSettings(&$o, $type_code)
	{
		$settings['start_tag'] = array_get_index($_POST, $type_code.'_start_tag', '');
		$settings['end_tag'] = array_get_index($_POST, $type_code.'_end_tag', '');
		$settings['paint_method'] = array_get_index($_POST, $type_code.'_paint_method', 'raw');

		return $settings;

	}//end processStandardPageSettings()


}//end class


?>
