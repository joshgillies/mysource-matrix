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
* $Id: cron_job_future_lineage_edit_fns.inc,v 1.7 2004/03/17 06:11:29 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Future_Lineage_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Lineage_Edit_Fns extends Cron_Job_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Future_Lineage_Edit_Fns()
	{
		$this->Cron_Job_Edit_Fns();

	}//end constructor


	/**
	* Prints the tasks interface in a one line format
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInLineBackend(&$asset, &$o, $prefix)
	{
		$major = &$asset->getAssetInLink('major');
		$minor = &$asset->getAssetInLink('minor');

		$o->openSection(($asset->id) ? $asset->name : 'New Future Linking');
		$o->openField('Delete Link');
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'minor');
		$link_options = Array(0 => '');
		foreach($links as $link) {
			$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
			if (is_null($parent)) continue;
			$link_options[$link['linkid']] = 'Link #'.$link['linkid'].' - Parent : '.$parent->name.' (#'.$parent->id.')';
		}
		combo_box($prefix.'_delete_linkid', $link_options, false, $asset->attr('delete_linkid'));


		$o->openField('New Link Type');
		$link_info = $asset->attr('link_info');
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$editable_link_types = Array(SQ_LINK_TYPE_1 => link_type_name(SQ_LINK_TYPE_1),
									 SQ_LINK_TYPE_2 => link_type_name(SQ_LINK_TYPE_2)
									);
		$link_type = (isset($link_info['link_type'])) ? $link_info['link_type'] : '';
		combo_box($prefix.'_link_type', $editable_link_types, false, $link_type);


		$o->openField('New Link Parent');
		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_major_assetid', ((is_null($major)) ? 0 : $major->id));
		} else {
			if (is_null($major)) {
				echo '<a href="'.$major->getBackendHref().'">'.$major->name.' (#'.$major->id.')</a>';
			} else {
				echo 'Not Set';
			}
		}

		$o->openField('At');
		$this->_paintWhenBox($asset, $o, $prefix);
		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$o->note('The Cron Manager runs every '.$cron_mgr->readableRefreshTime().'.<br>The next time that it will run will be '.$GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun()));

		$o->closeSection();

		return true;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInLineBackend(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_delete_linkid']) && (empty($_POST[$prefix.'_link_type']) || empty($_POST[$prefix.'_major_assetid']['assetid']))) return false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!empty($_POST[$prefix.'_delete_linkid'])) {
			if (!$asset->setAttrValue('delete_linkid', $_POST[$prefix.'_delete_linkid'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		if (!empty($_POST[$prefix.'_major_assetid']['assetid'])) {
			$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_major_assetid']['assetid']);
			if (is_null($major))  {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			$link_info = $asset->attr('link_info');
			$link_info['link_type'] = $_POST[$prefix.'_link_type'];
			if (!$asset->setAttrValue('link_info', $link_info) || !$asset->setAssetInLink($major, 'major')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		if (!$asset->saveAttributes() || !$this->_processWhenBox($asset, $o, $prefix)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processInLineBackend()


}//end class
?>
