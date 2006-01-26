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
* $Id: bodycopy_container.inc,v 1.31 2006/01/26 22:34:03 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Bodycopy_Container
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.31 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Container extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy_Container($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Returns linkid if successful or FALSE on failure
	*
	* @param array	&$link		information used to create the initial link
	* @param array	$bc_data	bodycopy data submitted to help create this div
	*
	* @return mixed int|boolean
	* @access public
	*/
	function create(&$link, $bc_data=Array())
	{
		// store this data for later use during the create process in _createAdditional()
		$this->_tmp['bodycopy_create_data'] = $bc_data;

		return parent::create($link);

	}//end create()


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Bodycopy containers ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Prints out the backend interface for the asset. Used for displaying
	* the div properties screen if needed.
	*
	* @param object	&$o	The Backend Outputter instance
	*
	* @return void
	* @access public
	*/
	function paintBackend(&$o)
	{

		if (!isset($_REQUEST['asset_ei_screen'])) {
			return parent::paintBackend($o);
		}

		switch ($_REQUEST['asset_ei_screen']) {
			case 'show_popup' :
				// Find out where the popups are located. (under bodycopy)
				$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo('bodycopy');
				$bodycopy_dir = SQ_SYSTEM_ROOT.'/'.$type_info['dir'];
				require_once $bodycopy_dir.'/popups/'.trim($_REQUEST['popup_file']);
				exit();
			default :
				return parent::paintBackend($o);

		}//end switch

	}//end paintBackend()


	/**
	* Saves a copy of this asset at this point in time
	*
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		if (!parent::saveSystemVersion()) return FALSE;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// save a version of the container as it appears now
		$edit = $this->getEditFns();
		if (!$edit->generateContentFile($this)) {
			trigger_localised_error('CORE0047', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// save the current notice links
		$replace_assetids = Array();
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);

		foreach ($notice_links as $link) {
			$replace_assetids[] = $link['minorid'];
		}

		// save the links we currently use (./?a=xx)
		if (!string_to_file(serialize($replace_assetids), $this->data_path.'/.sq_system/.notice_links')) {
			trigger_localised_error('CORE0048', E_USER_WARNING, $this->name);
			return FALSE;
		}

		return TRUE;

	}//end saveSystemVersion()


	/**
	* Processes the changes required when a new status is set
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parent	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parent=TRUE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$old_status = $this->status;
		if (!parent::processStatusChange($new_status, $update_parent)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// if we have just gone live from safe editing, we need to
		// create our notice links again so we dont have leftovers from
		// when we were safe editing and thus not deleting links we removed
		// Same thing goes cancelling a safe edit (going back to live)
		if ($this->status == SQ_STATUS_LIVE && ($old_status & SQ_SC_STATUS_SAFE_EDITING)) {
			$edit = $this->getEditFns();
			if (!$edit->createContentLinks($this)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end processStatusChange()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusively
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a content type then we need to make it a dependant link
		if ($side_of_link == 'major' && is_a($asset, 'content_type') && $dependant != '1') {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (!parent::deleteLink($linkid)) return FALSE;

		// link has been deleted, check urls to see if this link
		// was a HREF or SRC etc and get rid of them
		ob_start();
			$edit_fns = $this->getEditFns();
			$edit_fns->paint($this, TRUE);
			$output = ob_get_contents();
		ob_end_clean();

		// NOTE: don't need to worry about shadow assets here because we never link to them anyway
		$e = '/\\.\\/\\?a=([0-9]+)/';
		$matches = Array();
		preg_match_all($e, $output, $matches);
		$replace_assetids = $matches[1];

		return TRUE;

	}//end deleteLink()


	/**
	* Returns an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_2	=> Array(
									'content_type'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
													   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		if ($link['link_type'] == SQ_LINK_NOTICE) {
			return 'Linked inside the content as a URL, image, or embedded movie';
		}
		return parent::describeLink($linkid);

	}//end describeLink()


	/**
	* Called by the design to print the body of this asset
	*
	* @param array	$keyword_replacements	some replacements for keywords in the content
	*
	* @return void
	* @access public
	*/
	function printBody($keyword_replacements=Array())
	{
		if (!$this->readAccess()) return;

		// create an array of urls that the content file is
		// going to try and use to work out file urls, link hrefs etc

		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
		$get_href_ids = $this->attr('shadow_links');

		foreach ($notice_links as $link) {
			$get_href_ids[] = $link['minorid'];
		}

		$sq_asset_url_list = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($get_href_ids));
		for (reset($sq_asset_url_list); NULL !== ($minorid = key($sq_asset_url_list)); next($sq_asset_url_list)) {
			if ($sq_asset_url_list[$minorid] == '') {
				$sq_asset_url_list[$minorid] = './?a='.$minorid;
			}
		}

		if (SQ_ROLLBACK_VIEW) {
			$edit = $this->getEditFns();
			ob_start();
				$edit->paint($this, FALSE, TRUE);
				$output = ob_get_contents();
			ob_end_clean();

			// replace ./?a=xxx style links within the content
			foreach ($sq_asset_url_list as $assetid => $href) {
				if (strpos($assetid, ':') !== FALSE) {
					$s = './?a='.$assetid.'$';
					$output = str_replace($s, $href, $output);
				} else {
					$e = "(/?\./\\?a=$assetid)([^0-9:])";
					$output = ereg_replace($e, $href.'\\2', $output);
				}
			}

			// replace keywords in the content
			$output = replace_keywords($output, $keyword_replacements);

			echo $output;
			return;
		}

		if (!file_exists($this->data_path.'/content_file.php')) {
			// generate the content file
			$edit = $this->getEditFns();
			$edit->generateContentFile($this);
		}

		if (!file_exists($this->data_path.'/content_file.php')) {
			trigger_localised_error('CORE0009', E_USER_WARNING, $this->name);
		} else {
			include($this->data_path.'/content_file.php');
		}

	}//end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Determine if there is no content in this container
	*
	* @return boolean
	* @access public
	*/
	function isEmpty()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$content_types = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'content_type', FALSE);
		foreach ($content_types as $type) {
			$ct =& $am->getAsset($type['minorid'], $type['minor_type_code']);
			if (!$ct->isEmpty()) return FALSE;
		}
		return TRUE;

	}//end isEmpty()


}//end class

?>
