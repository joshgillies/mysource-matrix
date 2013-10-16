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
* $Id: bodycopy_container.inc,v 1.59 2013/10/16 05:14:46 cupreti Exp $
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
* @version $Revision: 1.59 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

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
	public function create(Array &$link, Array $bc_data=Array())
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
	protected function _preCreateCheck(Array &$link)
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
	public function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Prints out the backend interface for the asset. Used for displaying
	* the div properties screen if needed.
	*
	* @param Backend_Outputter	$o	The Backend Outputter instance
	*
	* @return void
	* @access public
	*/
	public function paintBackend(Backend_Outputter $o)
	{
		if (!isset($_REQUEST['asset_ei_screen'])) {
			return parent::paintBackend($o);
		}

		switch ($_REQUEST['asset_ei_screen']) {
			case 'show_popup' :
				// Find out where the popups are located. (under bodycopy)
				$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo('bodycopy');				
				
				$include_file = trim($_REQUEST['popup_file']);
				$bodycopy_popup_dir = SQ_SYSTEM_ROOT.'/'.$type_info['dir'].'/popups/';
				$dir_listing = scandir($bodycopy_popup_dir);
				
				if (in_array($include_file, $dir_listing)) {
					require_once($bodycopy_popup_dir.$include_file);
				} 

				exit(0);
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
	public function saveSystemVersion()
	{
		if (!parent::saveSystemVersion()) return FALSE;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// Bug #5460: New DIVs created in Safe Edit showing on front end before going Live.
		// There is no content yet but the DIV markup shows up and can cause issues if CSS is applied.
		// So lets not copy the content file over to .sq_system directory.
		if (!isset($this->_tmp['skip_save_system_version_content_file'])) {
			// save a version of the container as it appears now
			$edit = $this->getEditFns();
			if (!$edit->generateContentFile($this)) {
				trigger_localised_error('CORE0047', E_USER_WARNING, $this->name);
				return FALSE;
			}
	
			$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			foreach ($contexts as $contextid => $context_data) {
				$filename_suffix = ((int)$contextid === 0) ? '' : '.'.$contextid;
				if (file_exists($this->data_path.'/content_file'.$filename_suffix.'.php') === TRUE) {
					$copy_usv = copy_file($this->data_path.'/content_file'.$filename_suffix.'.php', $this->data_path.'/.sq_system/.content_file'.$filename_suffix.'.php');
	
				}
			}
		} else {
			unset($this->_tmp['skip_save_system_version_content_file']);
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
	public function processStatusChange($new_status, $update_parent=TRUE)
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
	* @param Bodycopy_Container	$asset			the major or minor asset that we are linking to
	* @param string				$side_of_link	the side of the link we are on (major or minor)
	* @param string				&$link_type		the type of link we are creating
	* @param string				&$value			the value that is to be associated with the link
	* @param string				&$sort_order	the position in the links list that this link should take,
	* @param string				&$dependant		'0' / '1' on whether the this asset is dependant on the
	*											asset that will be linked by the new link
	* @param string				&$exclusive		'0' / '1' on whether the this asset is linked exclusively
	*											to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a content type then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Content_Type) && $dependant != '1' && ($link_type & SQ_SC_LINK_SIGNIFICANT)) {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Returns an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
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
	public function describeLink($linkid)
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
	* @param mixed(array|null)	$keyword_replacements	some replacements for keywords in the content,
	* 													pass an array of replacements or 
	* 													null for the container to attempt the replacements itself
	*
	* @return void
	* @access public
	*/
	public function printBody($keyword_replacements=NULL)
	{
		if (!$this->readAccess()) return;

		// if we don't have any keyword replacements passed as an argument,
		// check if we really don't have any keywords in the contents because in some rare cases,
		// such as an asset listing that lists bodycopies and use %asset_contents% in the type format, 
		// this function is called without an array that contains keyword replacements which makes all the
		// keywords disappear from the contents printed on the frontend
		if (is_null($keyword_replacements)) {
			$keyword_replacements = Array();
			// find if we have any keywords in the content
			ob_start();
				$edit = $this->getEditFns();
				$edit->paint($this, FALSE, TRUE);
				$temp_raw_content = ob_get_contents();
			ob_end_clean();
			$keywords = retrieve_keywords_replacements($temp_raw_content);

			// if we have keywords, attempt to get replacement values for them
			if (!empty($keywords)) {
				$dependant_parent_assetids = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id, '', FALSE, FALSE);

				// if we know the one and only dependant parent that itself is not a dependant
				// then we can replace both global and asset keywords
				if (count($dependant_parent_assetids) === 1) {
					$dependant_parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset(array_pop($dependant_parent_assetids));
					foreach ($keywords as $keyword) {
						$keyword_replacements[$keyword] = $dependant_parent_asset->getKeywordReplacement($keyword);
					}
				} else if (count($dependant_parent_assetids) > 1) {
					// if we found multiple dependant parents (by new linking Content DIV, etc) then
					// we can have global keywords later on by passing the keywords as they are but 
					// not asset keyword as we don't know which asset to invoke getKeywordReplacement on.
					foreach ($keywords as $keyword) {
						if (0 === strpos($keyword, 'globals_')) {
							$keyword_replacements[$keyword] = "%$keyword%";
						}
					}
				}
			}
		}
		
		// Escape the quotes in the keyword
		foreach($keyword_replacements as $keyword => $val) {
			$safe_keyword = str_replace('$', '\\$', str_replace('\n', '\\n', htmlentities($keyword, ENT_QUOTES)));
			if ($safe_keyword != $keyword) {
				$keyword_replacements[$safe_keyword] = $val;
				unset($keyword_replacements[$keyword]);
			}
		}
		
		$contextid = (int)$GLOBALS['SQ_SYSTEM']->getContextId();
		$context_suffix = ($contextid === 0) ? '' : ('.'.$contextid);
		$content_file_name = 'content_file'.$context_suffix.'.php';
		$default_content_file_name = 'content_file.php';

		if ($this->useSystemVersion()) {
			$safe_edit_file_name = $this->data_path.'/.'.$content_file_name;
			if (file_exists($safe_edit_file_name) === FALSE) {
				$safe_edit_file_name = $this->data_path.'/.'.$default_content_file_name;
			}
			if (file_exists($safe_edit_file_name) === TRUE) {
				// create an array of urls that the content file is
				// going to try and use to work out file urls, link hrefs etc
				$sq_asset_url_list = Array();

				$notice_links = unserialize(file_to_string($this->data_path.'/.notice_links'));
				$get_href_ids = $this->attr('shadow_links');

				foreach ($notice_links as $link_assetid) {
					$get_href_ids[] = $link_assetid;
				}

				$sq_asset_url_list = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($get_href_ids));
				for (reset($sq_asset_url_list); NULL !== ($minorid = key($sq_asset_url_list)); next($sq_asset_url_list)) {
					if ($sq_asset_url_list[$minorid] == '') {
						$sq_asset_url_list[$minorid] = './?a='.$minorid;
					}
				}

				ob_start();
					include($safe_edit_file_name);
					$content = ob_get_contents();
				ob_end_clean();
				replace_keywords($content, $this->keyword_replacements);

				// remove any sq_wysiwyg_* tags used as placeholders for plugins
				// such as Embed Movie/Youtube but only on frontend
				if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
					$content = preg_replace('|<sq_wysiwyg_([^ >]*)([^>]*)>(.*)</sq_wysiwyg_\1>|si', '$3', $content);
				}

				echo $content;
			}
		} else {

			if (!isset($this->_tmp['asset_url_list'])) {

				// create an array of urls that the content file is
				// going to try and use to work out file urls, link hrefs etc

				$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
				$get_href_ids = $this->attr('shadow_links');

				foreach ($notice_links as $link) {
					$get_href_ids[] = $link['minorid'];
				}

				$this->_tmp['asset_url_list'] = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($get_href_ids));
				for (reset($this->_tmp['asset_url_list']); NULL !== ($minorid = key($this->_tmp['asset_url_list'])); next($this->_tmp['asset_url_list'])) {
					if ($this->_tmp['asset_url_list'][$minorid] == '') {
						$this->_tmp['asset_url_list'][$minorid] = './?a='.$minorid;
					}
				}

			}

			$sq_asset_url_list =& $this->_tmp['asset_url_list'];

			if (SQ_ROLLBACK_VIEW) {
				$edit = $this->getEditFns();
				ob_start();
					$edit->paint($this, FALSE, FALSE);
					$output = ob_get_contents();
					ob_end_clean();

				// replace ./?a=xxx style links within the content
				foreach ($sq_asset_url_list as $assetid => $href) {
					if (strpos($assetid, ':') !== FALSE) {
						$s = './?a='.$assetid.'$';
						$output = str_replace($s, $href, $output);
					} else {
						$e = "!(/?\./\\?a=$assetid)([^0-9:])!";
						$output = preg_replace($e, $href.'\\2', $output);
					}
				}

				// replace keywords in the content
				replace_keywords($output, $keyword_replacements);

				// remove any sq_wysiwyg_* tags used as placeholders for plugins
				// such as Embed Movie/Youtube but only on frontend
				if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
					$output = preg_replace('|<sq_wysiwyg_([^ >]*)([^>]*)>(.*)</sq_wysiwyg_\1>|si', '$3', $output);
				}

				echo $output;
				return;
			}

			if (!is_file($this->data_path.'/'.$content_file_name)) {
				// generate the content file
				$edit = $this->getEditFns();
				$edit->generateContentFile($this);
			}

			if (!is_file($this->data_path.'/'.$content_file_name)) {
				trigger_localised_error('CORE0009', E_USER_WARNING, $this->name);
			} else {

				ob_start();
					include($this->data_path.'/'.$content_file_name);
					$content = ob_get_contents();
				ob_end_clean();

				// remove any sq_wysiwyg_* tags used as placeholders for plugins
				// such as Embed Movie/Youtube but only on frontend
				if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
					$content = preg_replace('|<sq_wysiwyg_([^ >]*)([^>]*)>(.*)</sq_wysiwyg_\1>|si', '$3', $content);
				}

				echo $content;
			}
		}//end else

	}//end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Determine if there is no content in this container
	*
	* @return boolean
	* @access public
	*/
	public function isEmpty()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$content_types = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'content_type', FALSE);
		foreach ($content_types as $type) {
			$ct = $am->getAsset($type['minorid'], $type['minor_type_code']);
			if (!$ct->isEmpty()) return FALSE;
		}
		return TRUE;

	}//end isEmpty()


}//end class

?>
