<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: cron_job_future_lineage.inc,v 1.5 2003/11/18 15:37:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Future_Lineage
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Lineage extends Cron_Job
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Job_Future_Lineage($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Cron_Job($assetid);
	}


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$linkid = parent::create($link);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return 0;
		}

		if (!empty($this->_tmp['asset_in_link'])) {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return 0;
			}

			foreach($this->_tmp['asset_in_link'] as $side_of_link => $link) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($asset) || !$this->setAssetInLink($asset, $side_of_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$this->id = 0;
					return 0;
				}
			}// end foreach
			unset($this->_tmp['asset_in_link']);
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links');

		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		$minor = &$this->getAssetInLink('minor');
		if (is_null($minor)) {
			return 'Asset-less Future Lineage Cron Job';

		} elseif ($short_name) {
			return 'FL for "'.$minor->short_name.'" (#'.$minor->id.')';

		} else {
			return 'Future Lineage for "'.$minor->name.'" (#'.$minor->id.')';

		}// end if

	}//end _getName();


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		// Basically because we can be altering the lineage of any asset type
		// we need to make sure that we can cope with the possibility of
		// changing the lineage of a type that is used by the parent class
		// EG, 'user' for the running user
		for(reset($links[SQ_LINK_NOTICE]); (NULL !== ($type = key($links[SQ_LINK_NOTICE]))); next($links[SQ_LINK_NOTICE])) {
			if ($links[SQ_LINK_NOTICE][$type]['card'] != 'M') {
				$links[SQ_LINK_NOTICE][$type]['card'] = (int) $links[SQ_LINK_NOTICE][$type]['card'] + 1;
			}
		}

		// make sure that we can link to every asset
		if (empty($links[SQ_LINK_NOTICE]['asset'])) {
			$links[SQ_LINK_NOTICE]['asset'] = Array('card' => 1);
		}

		return $links;

	}//end _getAllowedLinks()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	* For Cron Jobs this can only be done by the running user or a sys admin
	* For Future Lineage Jobs, admins of the minor asset to update can also remove jobs
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		if (parent::canDelete()) return true;

		$minor = &$this->getAssetInLink('minor');
		if (!is_null($minor)) return $minor->adminAccess('');
		return false;

	}//end canDelete()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if ($name == 'type') {
			trigger_error('You can\'t set the type of a future lineage cron job it can only be a one off event', E_USER_NOTICE);
			return false;
		}// end if

		return parent::setAttrValue($name, $value);

	}// end setAttrValue()


	/**
	* Sets the asset that this cron job will be altering the status on
	*
	* @param object Asset	$asset			a reference to the asset
	* @param string			$side_of_link	the side of the link that this asset is to be - [major|minor]
	*
	* @return boolean
	* @access public
	*/
	function setAssetInLink(&$asset, $side_of_link)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor') {
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_WARNING);
			return false;
		}

		if (!is_a($asset, 'asset')) {
			trigger_error('Unable to set asset to update, the asset must be a asset object', E_asset_WARNING);
			return false;
		}

		if ($this->id) {
			return (bool) $this->createLink($asset, SQ_LINK_NOTICE, $side_of_link.'_asset');
		} else {
			if (!isset($this->_tmp['asset_in_link'])) $this->_tmp['asset_in_link'] = Array();
			$this->_tmp['asset_in_link'][$side_of_link] = Array('minorid' => $asset->id, 'minor_type_code' => $asset->type());
			return true;
		}

	}// end setAssetInLink()


	/**
	* Gets the user that this cron job is to run as while executing
	*
	* @param string	$side_of_link	the side of the link that this asset is to be - [major|minor]
	*
	* @return mixed object Asset or NULL on error
	* @access public
	*/
	function &getAssetInLink($side_of_link)
	{
		$null = null; // because we need to return by ref
		if ($side_of_link != 'major' && $side_of_link != 'minor') {
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_WARNING);
			return $null;
		}

		if ($this->id) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'asset', false, $side_of_link.'_asset');
		} else {
			$link = (isset($this->_tmp['asset_in_link'][$side_of_link])) ? $this->_tmp['asset_in_link'][$side_of_link] : Array();
		}

		if (empty($link)) return $null;
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

	}// end getAssetInLink()


	/**
	* Returns a description of what this future lineage job is going to do
	*
	* @return string
	* @access public
	*/
	function readableDescription()
	{
		$minor = &$this->getAssetInLink('minor');
		if (is_null($minor)) return 'No Minor Asset Found';

		$str = '';

		if ($this->attr('delete_linkid')) {
			$str .= 'Deleting Link Id #'.$this->attr('delete_linkid');
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attr('delete_linkid'));
			if (!empty($link)) {
				$old_major = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
				if (!is_null($old_major)) {
					$str .= ' - '.$minor->name.' (#'.$minor->id.') from under '.$old_major->name.' (#'.$old_major->id.')';
				}
			}
		} 

		$new_major = &$this->getAssetInLink('major');

		if ($this->attr('delete_linkid') && !is_null($new_major)) $str .= "\n";

		if (!is_null($new_major)) {
			$str .= 'Creating new link for '.$minor->name.' (#'.$minor->id.') under '.$new_major->name.' (#'.$new_major->id.')';
		}
		
		return $str;

	}// readableDescription()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* @param string	$msg	accepted by reference, used to return any messages about completion etc from _exec
	*
	* @return int	bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	* @see Cron_Job::_exec(), Cron_Job::run()
	* @access private
	*/
	function _exec(&$msg)
	{
		// we can be removed because the error is beyound our control
		$failed_ret_val = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

		// need to get this here because we are could be deleting the link below
		$desc = $this->readableDescription();

		$minor = &$this->getAssetInLink('minor');
		if (is_null($minor)) {
			trigger_error('Couldn\'t find Minor Asset to use to update the lineage', E_USER_WARNING);
			return $failed_ret_val;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($this->attr('delete_linkid')) {
			$delete_ok = false;
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attr('delete_linkid'));
			if (!empty($link)) {
				$old_major = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
				if (!is_null($old_major) && $GLOBALS['SQ_SYSTEM']->am->acquireLock($old_major->id, 'links')) {
					$delete_ok = $old_major->deleteLink($this->attr('delete_linkid'));
				}
			}

			if (!$delete_ok) {
				trigger_error('Delete of Link # '.$this->attr('delete_linkid').' Failed', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $failed_ret_val;
			}

		}// end if

		$new_major = &$this->getAssetInLink('major');
		if (!is_null($new_major)) {

			$link_info = $this->attr('link_info');
			if (!isset($link_info['value']))		$link_info['value']			= '';
			if (!isset($link_info['sort_order']))	$link_info['sort_order']	= null;
			if (!isset($link_info['dependant']))	$link_info['dependant']		= '0';
			if (!isset($link_info['exclusive']))	$link_info['exclusive']		= '0';


			$linkid = $new_major->createLink($minor,
												$link_info['link_type'],
												$link_info['value'],
												$link_info['sort_order'],
												$link_info['dependant'],
												$link_info['exclusive']);

			$create_ok = ($linkid > 0);

			if (!$create_ok) {
				trigger_error('Create of New Link under '.$new_major->name.' (#'.$new_major->id.') Failed', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $failed_ret_val;
			}

		}// endif


		require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_update_lookups.inc';
		$init_hipo = new HIPO_Job_Update_Lookups();
		$init_hipo->setRunningVars(Array('assetids' => Array($minor->id)));
		$code_name = $init_hipo->initialise();
		if (!$code_name) {
			trigger_error('Future Lineage of "'.$minor->name.'" (#'.$minor->id.') Failed, HIPO was not able to initialise', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $failed_ret_val;
		} // end if

		require_once 'XML/Tree.php';
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$attrs = Array(
					'command' => 'start',
					'userid'  => $GLOBALS['SQ_SYSTEM']->currentUserId(),
					'source_code_name' => $code_name,
				);
		$input = new XML_Tree();
		$input_root = &$input->addRoot('hipo_job', '', $attrs);
		$output = new XML_Tree();
		$output_root = &$output->addRoot('output');

		if (!$hh->prepareProcessServer($input_root, $output_root, '')) {
			if ($ack->root->name == 'error') {
				trigger_error('HIPO Herder Error : '.$ack->root->content, E_USER_WARNING);
			} else {
				trigger_error('Unexpected return from the HIPO Server : '.$ack->get(), E_USER_WARNING);
			}// end switch
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $failed_ret_val;
		}
		if (!$hh->processServer()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $failed_ret_val;
		}

		$msg = "Successful in \n".$desc;
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		// All OK, and we can be removed because we have done our one off task
		return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;

	}// end _exec()


	/**
	* Returns an array of active future lineage jobs that are to be run on the passed asset
	* returned in chronological order
	*
	* @param object Asset	$asset	the asset to get the future status jobs for
	*
	* @return Array() returns an array of references to the Future Lineage jobs
	* @access public
	* @static
	*/
	function &getActiveJobs(&$asset)
	{
		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) {
			$blank = Array();
			return $blank;
		}

		// get all active future lineage jobs
		$all_jobs = &$cron_mgr->getJobs(__CLASS__, false);

		$dates = Array();
		for($i = 0, $total = count($all_jobs); $i < $total; $i++) {

			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($all_jobs[$i]->id, SQ_LINK_NOTICE, 'asset', false, 'minor_asset');
			if (!empty($link) && $link['minorid'] == $asset->id) {
				$dates[$i] = $all_jobs[$i]->attr('when');
			}

		}// end for

		asort($dates, SORT_STRING);
		$assets_jobs = Array();

		foreach($dates as $i => $date) {
			$assets_jobs[] = &$all_jobs[$i];
		}// end for

		return $assets_jobs;

	}// end getActiveJobs()


}//end class
?>
