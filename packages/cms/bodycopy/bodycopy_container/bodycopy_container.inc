<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_container.inc,v 1.1 2003/11/03 01:22:28 gsherwood Exp $
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
* @subpackage cms
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
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a content type then we need to make it a dependant link
		if (is_a($minor, 'content_type')) {
			$dependant = '1';
		}
		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}// end createLink()


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
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
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
			// generate the contents file
			$edit = $this->getEditFns();
			$edit->generateContentFile($this);
		}

		if (!file_exists($this->data_path.'/content_file.php')) {
			trigger_error('Contents file failed to generate for bodycopy container '.$this->name, E_USER_WARNING);
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