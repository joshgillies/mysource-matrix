<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_sub_asset_listing.inc,v 1.7 2003/10/13 23:05:43 gsherwood Exp $
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
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
			'type_formats' => 'folder',
			'asset_formats' => 'folder',
			'page_contents' => 'Bodycopy',
		);
		$type_formats = null;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'dependant' => 	1, 'exclusive' => 1);
		
			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
			if ($name == 'type_formats') {
				$type_formats = $asset;
			}
		}
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$type_formats, 'value' => 'default_format' ,'link_type' => SQ_LINK_TYPE_2, 'dependant' => 	1, 'exclusive' => 1);
		$asset->setAttrValue('name', 'Default Format');
		
		if (!$asset->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

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
		$bodycopies = Array('default_format', 'page_contents');
		foreach ($bodycopies as $name) {
			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', $name, '1', '1');
			if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the '.ucwords(str_replace('_', ' ', $name)).' and the sub asset listing page';
		}
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
	* returns a reference to a folder linked to this asset determined by the passe in value on the link
	*
	* @param string $type the value on the link for this folder
	*
	* @access public
	* @return &asset folder
	*/
	function &getFolder($type='type_formats')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, $type);
		if (empty($link)) return null;

		$folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return null; 

		return $folder;

	} // end getFolder


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (empty($bodycopy_link)) return false;

		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($bodycopy)) return false; 

		ob_start();
		$bodycopy->printBody();
		$html = ob_get_contents();
		ob_end_clean();

		$body = &$this->getBodyReplacement();
		$html = str_replace('%sub_asset_listing%', $body, $html);
		
		echo $html;

	}// end printBody() 


	/**
	* returns a replacement for the body keyword in the page contents bodycopy
	*
	* @access public
	* @return &string
	*/
	function &getBodyReplacement() 
	{
		$wanted_types = $this->attr('types');
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV, $wanted_types, false);
		$customised = $this->attr('customised');

		if (empty($children)) return false;

		$keywords			= Array();								// an array of keywords/replacements per asset base
		$formats			= Array();								// an array of customised formats html
		$default_format		= '';									// the default format html
		$is_default			= Array();								// array of types using the default format
		$folder				= &$this->getFolder('type_formats');	// the folder asset for type formats
		$type_folder_id		= $folder->id;							// the format types folder id
		$global_contents	= '';									// the entire contents of the SAL
		$mm					= null;									// metadata manager

		foreach ($children as $info) {
			$assetid = $info['minorid'];
			$type_code = $info['minor_type_code'];
			$contents = ''; // temp contents for this asset

			// check to see if this is a bodycopy that is used for a format
			if (in_array(strtolower($info['value']), $wanted_types)) continue;
			if (strtolower($info['value'] == 'default_format')) continue;
			if (strtolower($info['value'] == 'page_contents')) continue;

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
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($type_folder_id, SQ_LINK_TYPE_2, 'bodycopy', true, $type_code);
					// maybe they deleted the bodycopy
					if (!$link) {
						$key = array_search($type_code, $customised, true);
						unset($customised[$key]);
						$is_default[] = $type_code;
					} else {
						$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
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
			} // end if

			if (in_array($type_code, $is_default)) {
				if (!$default_format) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($type_folder_id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
					$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'bodycopy');
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
			if (preg_match_all('|%asset_metadata_([^%]+)%|is', $contents, $matches)) {
				if (is_null($mm)) $mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				// get all the metadata keywords for this asset
				$replacements = $mm->getMetadataFieldValues($assetid, $matches[1]);
				
				foreach ($replacements as $field => $value) {
					$keywords['asset_metadata_'.$field]['value'] = $value;
				}
			}
			foreach ($keywords as $keyword => $info) {
				$contents = str_replace('%'.$keyword.'%', $info['value'], $contents);
			}
			$global_contents .= $contents;
		
		} // end foreach

		// if we have any blank keywords, replace them with nothing
		$global_contents = preg_replace('|%[^%\W]+%|', '', $global_contents);

		return $global_contents;

	}// end getBodyReplacement()

}//end class
?>
