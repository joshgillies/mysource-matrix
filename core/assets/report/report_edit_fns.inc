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
* $Id: report_edit_fns.inc,v 1.20 2012/08/30 01:09:09 ewang Exp $
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
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint the interface for regenerating the report
	*
	* @param Report				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintRegenerate(Report $asset, Backend_Outputter $o, $prefix)
	{
		$report_path = $asset->data_path.'/report.xml';
		if (!is_file($report_path)) {
			$o->note(translate('This report has not been generated'));

		} else {
			$o->note(sprintf(translate('<b>This report has been generated</b> <br />Date: %1$s<br />User: %2$s'), $asset->attr('generated_date'), $asset->attr('generated_user')));

		}

		if ($asset->writeAccess('attributes')) {
			check_box($prefix.'_regenerate');
			label(translate('Generate the latest version of this report'), $prefix.'_regenerate');

			return TRUE;
		}
		return FALSE;

	}//end paintRegenerate()


	/**
	* Processes the interface for regenerating the report
	*
	* @param Report				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processRegenerate(Report $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_regenerate']) || !$_POST[$prefix.'_regenerate']) {
			return FALSE;
		}

		$user = $GLOBALS['SQ_SYSTEM']->user;
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assetid'			=> $asset->id,
					'userid'			=> $user->id,
					'generated_date'	=> date('F j, Y, g:i a'),
				);
		$hh->queueHipo('hipo_job_generate_report', $vars);

		if (!$asset->writeAccess('links')) return FALSE;

		$asset->setAttrValue('generated_date', date('F j, Y, g:i a'));
		$asset->setAttrValue('generated_user', $user->name.' (#'.$user->id.')');

		return TRUE;

	}//end processRegenerate()


	/**
	* Paints the root node selection box
	*
	* @param Report				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintRootNode(Report $asset, Backend_Outputter $o, $prefix)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'root');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			if (!empty($root_link)) {
				echo get_asset_tag_line($root_link['minorid']);
			} else {
				echo translate('No root node has been selected');

			}
		} else {
			asset_finder($prefix.'_rootid', (!empty($root_link)) ? $root_link['minorid'] : '');
		}

		return $write_access;

	}//end paintRootNode()


	/**
	* Processes the value input from root node selection box
	*
	* @param Report				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function processRootNode(Report $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rootid']['assetid']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version']) ) {
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
			$root = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_root);
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
	* @param Report	$asset	the asset whose interface we are painting
	*
	* @return string
	* @access public
	*/
	public function getRootAssetid(Report $asset)
	{
		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'root');
		if (!empty($root_link)) return $root_link['minorid'];

		// the whole system
		return '1';

	}//end getRootAssetid()


	/**
	* Paint the interface for viewing a report
	*
	* @param Report				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintReport(Report $asset, Backend_Outputter $o, $prefix)
	{
		echo translate('This report does not contain a visual component');

		return FALSE;

	}//end paintReport()


	/**
	* Processes the interface for viewing a report
	*
	* @param Report				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the backend outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processReport(Report $asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end processReport()


	/**
	* Generate this report until complete
	*
	* @param HIPO_Job	$job		a reference to the HIPO job processing this report
	* @param array		&$step_data	a reference to the array of information about
	*								the current step of the HIPO job that is running this report
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	public function generateReport(HIPO_Job $job, Array &$step_data, $prefix)
	{
		trigger_localised_error('CORE0079', E_USER_WARNING);
		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;
		return TRUE;

	}//end generateReport()


}//end class

?>
