<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: report_edit_fns.inc,v 1.13 2006/01/26 22:34:05 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Report_Edit_Fns
*
* Purpose
*	Base Edit Interface for all reports
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Report_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint the interface for regenerating the report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintRegenerate(&$asset, &$o, $prefix)
	{
		$report_path = $asset->data_path.'/report.xml';
		if (!is_file($report_path)) {
			$o->note(translate('report_not_generated'));
		} else {
			$o->note(translate('core_report_last_generated', $asset->attr('generated_date'), $asset->attr('generated_user')));
		}

		if ($asset->writeAccess('attributes')) {
			check_box($prefix.'_regenerate');
			label(translate('core_generate_report'), $prefix.'_regenerate');
			return TRUE;
		}
		return FALSE;

	}//end paintRegenerate()


	/**
	* Processes the interface for regenerating the report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processRegenerate(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_regenerate']) || !$_POST[$prefix.'_regenerate']) {
			return FALSE;
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetid' => $asset->id);
		$hh->queueHipo('hipo_job_generate_report', $vars);

		if (!$asset->writeAccess('links')) return FALSE;

		$user = $GLOBALS['SQ_SYSTEM']->user;
		$asset->setAttrValue('generated_date', date('F j, Y, g:i a'));
		$asset->setAttrValue('generated_user', $user->name.' (#'.$user->id.')');

		return TRUE;

	}//end processRegenerate()


	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'root');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			if (!empty($root_link)) {
				echo get_asset_tag_line($root_link['minorid']);
			} else {
				echo translate('core_no_selectes_root_node');
			}
		} else {
			asset_finder($prefix.'_rootid', (!empty($root_link)) ? $root_link['minorid'] : '');
		}

		return $write_access;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid'])) {
			return FALSE;
		}
		if (!$asset->writeAccess('links')) return FALSE;

		$new_root  = $_POST[$prefix.'_rootid']['assetid'];

		// getting existing root link
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'root');

		// dont process anything if the root node has not changed
		if (!empty($root_link) && $root_link['minorid'] == $new_root) {
			return FALSE;
		}

		$ok = TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old root link if any found...
		if (!empty($root_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($root_link['linkid'])) {
			$ok = FALSE;
		} else if ($new_root) {
			$root =& $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
			if (!is_null($root)) {
				if (!$asset->createLink($root, SQ_LINK_NOTICE, 'root')) {
					$ok = FALSE;
					trigger_localised_error('CORE0029', E_USER_WARNING, $root->name);
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processRootNode()


	/**
	* Get the assetid of the root node
	*
	* @param object	&$asset	the asset whose interface we are painting
	*
	* @return string
	* @access public
	*/
	function getRootAssetid(&$asset)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'root');
		if (!empty($root_link)) return $root_link['minorid'];

		// the whole system
		return '1';

	}//end getRootAssetid()


	/**
	* Paint the interface for viewing a report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the backend outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReport(&$asset, &$o, $prefix)
	{
		echo translate('core_no_visual_report');

	}//end paintReport()


	/**
	* Processes the interface for viewing a report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the backend outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processReport(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processReport()


	/**
	* Generate this report until complete
	*
	* @param object	&$job		a reference to the HIPO job processing this report
	* @param array	&$step_data	a reference to the array of information about
	*							the current step of the HIPO job that is running this report
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{
		trigger_localised_error('CORE0079', E_USER_WARNING);
		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;
		return TRUE;

	}//end generateReport()


}//end class

?>