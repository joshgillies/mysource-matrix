<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_sub_asset_listing.inc,v 1.2 2003/10/10 04:28:14 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';

/**
* Page_Sub_Asset_Listing
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Sub_Asset_Listing extends Page
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Sub_Asset_Listing($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


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
			return false;
		}

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'default_format' ,'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		$bodycopy->setAttrValue('name', 'Default Format');
		if (!$bodycopy->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


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
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}// end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', 'default_format', '1', '1');
		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the standard page';

		return parent::canDeleteLink($linkid);

	}//end canDeleteLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 1, 'exclusive' => true);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		if (isset($_GET['sq_from_frontend'])) {
			// get the bodycopy attached to us
			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
			$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link[0]['minorid'], 'bodycopy');
			if (is_null($bodycopy)) {
				trigger_error('Could not locate the bodycopy for standard page "'.$this->name.'"', E_USER_ERROR);
				return false;
			}
			$o->addFormActionGetVar('assetid', $bodycopy->id);
			$bodycopy->paintBackend($o);
		} else {
			parent::paintBackend($o);
		}

	}// end paintBackend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
		$wanted_types = $this->attr('types');
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV, $wanted_types, false);
		$customised = $this->attr('customised');

		if (empty($children)) return false;

		$keywords = Array();
		$formats = Array();
		$default_format = '';
		$is_default = Array();

		foreach ($children as $info) {
			$assetid = $info['minorid'];
			$type_code = $info['minor_type_code'];
			$contents = '';

			// check to see if this is a bodycopy that is used for a format
			if (in_array(strtolower($info['value']), $wanted_types)) continue;
			if (strtolower($info['value'] == 'default_format')) continue;

			// we need to work out if this child has an explicit format defined for it
			// or if its relying on its ansestor's format
			if (!in_array($type_code, $wanted_types)) {
				foreach ($wanted_types as $wt) {
					$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($wt);
					if (in_array($type_code, $desc)) $type_code = $wt;
				}
			}

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$keywords = $asset->getAssetKeywords();
			if (!in_array($type_code, $formats)) {
				if (in_array($type_code, $customised)) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type_code);
					// maybe they deleted the bodycopy
					if (!$link) {
						$key = array_search($type_code, $customised, true);
						unset($customised[$key]);
						$is_default[] = $type_code;
					} else {
						$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
						if (!is_null($bodycopy)) {
							ob_start();
							$bodycopy->printBody();
							$formats[$type_code] = ob_get_contents();
							ob_end_clean();
						}
					}
				} else {
					$is_default[] = $type_code;
				}
			}

			if (in_array($type_code, $is_default)) {
				if (!$default_format) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
					$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
					if (!is_null($bodycopy)) {
						ob_start();
						$bodycopy->printBody();
						$default_format = ob_get_contents();
						ob_end_clean();
					}
				}
				$contents = $default_format;
			} else {
				$contents = $formats[$type_code];
			}
			foreach ($keywords as $keyword => $info) {

				$contents = str_replace('%'.$keyword.'%', $info['value'], $contents);
			}
			echo html_entity_decode($contents);
		}

	}// end printBody()

}//end class
?>
