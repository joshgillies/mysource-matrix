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
* $Id: bodycopy_container.inc,v 1.6 2004/02/18 18:19:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Bodycopy_Container
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Saves a copy of this asset at this point in time
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		if (!parent::saveSystemVersion()) return false;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// save a version of the container as it appears now
		$edit = $this->getEditFns();
		if (!$edit->generateContentFile($this)) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create content file', E_USER_WARNING);
			return false;
		}

		// save the current notice links
		$replace_assetids = Array();
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
		foreach ($notice_links as $link) $replace_assetids[] = $link['minorid'];

		// save the links we currently use (./?a=xx)
		if (!string_to_file(serialize($replace_assetids), $this->data_path.'/.sq_system/.notice_links')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create notice links file', E_USER_WARNING);
			return false;
		}

		return true;

	}//end saveSystemVersion()


	/**
	* Processes the changes required when a new status is set
	*
	* @param int	$new_status		the new status to set for this asset
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$old_status = $this->status;
		if (!parent::processStatusChange($new_status)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// if we have just gone live from safe editing, we need to
		// create our notice links again so we dont have leftovers from
		// when we were safe editing and thus not deleting links we removed
		// Same thing goes cancelling a safe edit (going back to live)
		if ($this->status == SQ_STATUS_LIVE && ($old_status & SQ_SC_STATUS_SAFE_EDITING)) {
			$edit = $this->getEditFns();
			if (!$edit->generateContentFile($this)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processStatusChange()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string			$side_of_link	the side of the link we are on (major or minor)
	* @param string			$link_type		the type of link we are creating
	* @param string			$value			the value that is to be associated with the link
	* @param string			$sort_order		the position in the links list that this link should take,
	* @param string			$dependant		'0' / '1' on whether the this asset is dependant on the
	*										asset that will be linked by the new link
	* @param string			$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*										to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a content type then we need to make it a dependant link
		if ($side_of_link == 'major' && is_a($asset, 'content_type') && $dependant != '1') {
			$dependant = '1';
			return true;
		}

		return false;

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
		if (!parent::deleteLink($linkid)) return false;
			
		// link has been deleted, check urls to see if this link
		// was a HREF or SRC etc and get rid of them
		ob_start();
		$edit_fns = $this->getEditFns();
		$edit_fns->paint($this, true);
		$output = ob_get_contents();
		ob_end_clean();

		$e = '/\\.\\/\\?a=([0-9]+)/';
		$matches = Array();
		preg_match_all($e, $output, $matches);
		$replace_assetids = $matches[1];

		if (in_array($link['minorid'], $replace_assetids)) {
			// we were using this asset... thanks very much... I'm telling...
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());

			$message = $ms->newMessage();
			$message->subject = 'Bodcopy Container Link Deleted';
			$message->body = 'User "'.$user->name.'" has deleted '.$asset->type().' "'.$asset->name.'" which was being linked to in container "'.$this->name.'". At least one URL or image is now broken in this container.';
			$message->to = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, SQ_PERMISSION_ADMIN, true, false);
			$message->from = 0; // a system message
			$message->send();
		}

		return true;

	}//end deleteLink()


	/**
	* Returns an array of all the permitted link types, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_2 => Array('content_type' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array('asset' => Array('card' => 'M', 'exclusive' => false))
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
	* @param Array $keyword_replacements some replacements for keywords in the content
	*
	* @access public
	*/
	function printBody($keyword_replacements=Array())
	{
		// create an array of urls that the content file is
		// going to try and use to work out file urls, link hrefs etc
		$sq_asset_url_list = Array();
		
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
		$get_href_ids = Array();
		foreach ($notice_links as $link) $get_href_ids[] = $link['minorid'];
		
		$hrefs = $GLOBALS['SQ_SYSTEM']->am->getAssetHref(array_unique($get_href_ids));
		foreach ($hrefs as $minorid => $href) {
			if ($href == '') $href = './?a='.$minorid;
			$sq_asset_url_list[$minorid] = $href;
		}

		if(SQ_ROLLBACK_VIEW) {
			$edit = $this->getEditFns();
			ob_start();
			$edit->paint($this, true);
			$output = ob_get_contents();
			ob_end_clean();

			foreach ($sq_asset_url_list as $assetid => $href) {
				$e = "(/?\./\\?a=$assetid)([^0-9]+)";
				$output = ereg_replace($e, $href.'\\2', $output);
			}
			echo $output;
			return;
		}

		if (!file_exists($this->data_path.'/content_file.php')) {
			// generate the content file
			$edit = $this->getEditFns();
			$edit->generateContentFile($this);
		}

		if (!file_exists($this->data_path.'/content_file.php')) {
			trigger_error('Content file failed to generate for bodycopy container '.$this->name, E_USER_WARNING);
		} else {
			include($this->data_path.'/content_file.php');
		}

	}// end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Determine if there is no content in this container
	*
	* @access public
	* @return boolean
	*/
	function isEmpty()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$content_types = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'content_type', false);
		foreach ($content_types as $type) {
			$ct = &$am->getAsset($type['minorid'], $type['minor_type_code']);
			if (!$ct->isEmpty()) {
				return false;
			}
		}
		return true;

	}// end isEmpty()

}//end class

?>