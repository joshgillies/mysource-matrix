<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_table.inc,v 1.42 2003/10/10 06:49:27 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once(dirname(__FILE__).'/bodycopy_table_cell.inc');

/**
* Bodycopy_Table
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Table extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy_Table($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link		information used to create the initial link
	* @param int		$num_rows	the number of rows to create
	* @param int		$num_cols	the number of columns to create
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link, $num_rows=1, $num_cols=1)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a BodyCopy Table with a blank name', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		for ($i = 0; $i < $num_rows; $i++) {
			$rows[$i] = Array();
			$rows[$i]['attributes'] = Array();
			$rows[$i]['cells']		= Array();
			for ($x = 0; $x < $num_cols; $x++) {
				$rows[$i]['cells'][$x]	= new Bodycopy_Table_Cell();
				// short and long names are the same to begin with
				if (!$rows[$i]['cells'][$x]->setContentType($this, 'wysiwyg', $i, $x)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$this->id = 0;
					return false;
				}
			}
		}

		// set row information
		$this->setAttrValue('rows',$rows);
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes') || !$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()


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

		// save a version of the table as it appears now
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
			$message->subject = 'Table Link Deleted';
			$message->body = 'User "'.$user->name.'" has deleted '.$asset->type().' "'.$asset->name.'" which was being linked to in table "'.$this->name.'". At least one URL or image is now broken in this table.';
			$message->to = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, SQ_PERMISSION_ADMIN, true, false);
			$message->from = 0; // a system message
			$message->send();
		}

		return true;

	}


	/**
	* Remaps existing assetids used by this asset to new ones
	*
	* @param Array	&$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return bool
	* @access public
	*/
	function remapAssetids($map)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (!parent::remapAssetids($map)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$old_assetid = array_search($this->id, $map);
		if ($old_assetid === FALSE) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}

		$old_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($old_assetid, $this->type());
		$our_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, 'content_type', false);
		
		if (is_null($old_asset) || empty($our_links)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}
		$rows = $this->attr('rows');

		for($i = 0; $i < count($rows); $i++) {
			// now print the cell contents
			for($j = 0; $j < count($rows[$i]['cells']); $j++) {
				$old_linkid = $rows[$i]['cells'][$j]->attributes['linkid'];
				$old_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($old_linkid, $old_asset->id);
				if (empty($old_link)) continue;

				if (isset($map[$old_link['minorid']])) {
					$new_assetid = $map[$old_link['minorid']];

					foreach($our_links as $our_link) {
						if ($our_link['minorid'] == $new_assetid) {
							$rows[$i]['cells'][$j]->attributes['linkid'] = $our_link['linkid'];
							break;
						}
					}// end for

				}// end if
			}// end for
		}// end for

		if ($this->setAttrValue('rows', $rows)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return true;
		}

	}// end remapAssetids()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
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
			if ($href == '') $href = "Javascript: alert('This link does not exist');";
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
			trigger_error('Contents file failed to generate for bodycopy table '.$this->name, E_USER_WARNING);
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
	* Determine if there is no content in this table
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