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
* $Id: calendar_event_multi_date.inc,v 1.18 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Multi-Date Calendar Event
*
* Purpose
* 		Allows multiple Single and Recurring Calendar instances to be combined under one event
*
* @author  Greg Sherwoood <greg@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Multi_Date extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Multi_Date($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	* Multi-Date Events must be supplied with a name prior to creation
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
			trigger_localised_error('CORE0083', sprintf(translate('Unable to create a %s with a blank name'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// Add web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
		if (!$this->saveWebPaths($good_paths)) return FALSE;

		// Create bodycopy
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 0);
		$bodycopy->setAttrValue('name', 'Page Contents');
		if (!$bodycopy->create($copy_link)) return FALSE;

		// Process pending children
		if (!empty($this->_tmp['pending_children'])) {
			foreach ($this->_tmp['pending_children'] as $entry) {
				if (!$entry['asset']->create($entry['link'])) {
					return FALSE;
				}
			}
		}

		$this->name = $this->_getName();
		$this->short_name = $this->_getName();

		return TRUE;

	}//end _createAdditional()


	/**
	* Perform any additional processing required during the cloning of components of this asset
	*
	* @return boolean
	* @access public
	*/
	public function cloneDependantAdditional(Asset $clone)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$return_value = TRUE;
		$value = TRUE;

		// Load the web paths for the design's components
		$components = $GLOBALS['SQ_SYSTEM']->am->getChildren($clone->id);
		foreach ($components as $componentid => $component) {
			// set web path for components
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($componentid);

			$initial_path = strtolower($asset->name);
			$valid_paths = make_valid_web_paths(Array($initial_path));
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($clone, $valid_paths, $asset->id, TRUE);
			$GLOBALS['SQ_SYSTEM']->acquireLock($asset->id, 'all');
			$return_value = $asset->saveWebPaths($good_paths);
			$value = (!$return_value) ? FALSE : TRUE;

			$return_value = $asset->updateLookups(FALSE);
			$value = (!$return_value) ? FALSE : TRUE;
			$GLOBALS['SQ_SYSTEM']->releaseLock($asset->id, 'all');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		}//end foreach

		return $value;

	}//end cloneDependantAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	* @see Asset::_getAllowLinks()
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['folder'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['data_record'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['folder'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['data_record'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_3]['calendar_event'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['calendar_event'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 1, 'exclusive' => FALSE);

		return $allowed;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		if ($contextid === 0) {
			$cache_key = 'default';
		} else {
			$cache_key = 'default.'.$contextid;
		}

		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$cached_contents = $cm->loadFromCache($this->id, $cache_key);

		// if the cache is empty, go ahead and regenrate a new version
		if ($cached_contents === FALSE) {
			// get the bodycopy here to avoid caching of any possible error messages
			$bodycopy = $this->getBodycopy();
			ob_start();
				// we only want to cache if we actually have a bodycopy
				if (!is_null($bodycopy)) {
					$bodycopy->printBody();
					$cm->saveToCache($this->id, $this->type(), $cache_key, ob_get_contents());
				}
			ob_end_flush();
		} else {
			echo $cached_contents;
		}

	}//end printBody()


	/**
	* Get the replacement for the short date keyword
	*
	* @return string
	* @access public
	*/
	function getEventShortDateKeywordReplacement()
	{
		return $this->attr('short_date');

	}//end getEventShortDateKeywordReplacement()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
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
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	* @see saveAttributes()
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if ($name == 'name') {
			$this->_tmp['name_updated'] = TRUE;
		}
		if ($name == 'short_date') {
			$this->_tmp['short_date_updated'] = TRUE;
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Return the bodycopy linked to this page
	*
	* @return object
	* @access public
	*/
	function &getBodycopy()
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, '', 'major');
		if (empty($link)) return $null;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $bodycopy;

	}//end getBodycopy()


	/**
	* Register an asset to be created as a child of this asset when this asset is created
	*
	* @param object	&$asset	The child asset that needs creating
	* @param array	$link	Details of the link to create the child with
	*
	* @return void
	* @access public
	*/
	function addPendingChild(&$asset, $link)
	{
		if ($this->id) {
			trigger_error('Cannot add pending child - parent is already created');
			return;
		}
		if (empty($this->_tmp['pending_children'])) {
			$this->_tmp['pending_children'] = Array();
		}
		$this->_tmp['pending_children'][] = Array('asset' => $asset, 'link' => $link);

	}//end addPendingChild()


	/**
	* Get replacements for keywords that are specific to this asset
	* @see Asset::getCustomKeywordReplacements()
	*
	* @param array	$keywords	the keywords to get replacements for
	*
	* @return array
	* @access public
	*/
	function getCustomKeywordReplacements($keywords, $invoke_backend = FALSE)
	{
		if (!is_array($keywords) || empty($keywords)) {
			return Array();
		}
		$print_commit_button = FALSE;

		$replacements = Array();
		foreach ($keywords as $keyword) {
			switch ($keyword) {
				case 'contents' :
					$bodycopy = $this->getBodycopy();
					if (is_null($bodycopy)) continue;
					$edit = $bodycopy->getEditFns();
					if (!$invoke_backend) {
						$o = $GLOBALS['SQ_SYSTEM']->backend->out ;
					} else {
						require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
						$o = new Backend_Outputter();
					}
					ob_start();
						if ($edit->paintBodycopy($bodycopy, $o, $bodycopy->getPrefix())) {
							$print_commit_button = TRUE;
						}
						$replacements[$keyword] = ob_get_contents();
					ob_end_clean();
				break;

				default:
					$replacements[$keyword] = '';
				break;
			}//end switch
		}//end foreach

		return Array('replacements' => $replacements, 'print_commit_button' => $print_commit_button);

	}//end getCustomKeywordReplacements()


	/**
	* Process anything that was previously printed for a custom keyword
	*
	* @param array	$keywords	the keywords that were originally printed
	*
	* @return boolean
	* @access public
	* @see Asset::processCustomKeywords()
	*/
	function processCustomKeywords($keywords)
	{
		if (!is_array($keywords) || empty($keywords)) {
			return Array();
		}

		foreach ($keywords as $keyword) {
			switch ($keyword) {
				case 'contents' :
					$bodycopy = $this->getBodycopy();
					if (is_null($bodycopy)) continue;
					$edit = $bodycopy->getEditFns();
					$edit->processBodycopy($bodycopy, $GLOBALS['SQ_SYSTEM']->backend->out, $bodycopy->getPrefix());
				break;
			}
		}

		return TRUE;

	}//end processCustomKeywords()


	/**
	* Provides extra checking and disable invalid  statud change on errors
	*
	* @param int	$new_status	the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parents as well
	*
	* @return boolean
	* @access public
	**/
	function processStatusChange($new_status, $update_parents)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		//make sure our data directory exist
		if (!create_directory($this->data_path)) {
			trigger_localised_error('CORE0049', sprintf(translate('Failed saving asset "%s" to cache, could not create restricted data directory'), $this->name), E_USER_WARNING);
			return FALSE;
		}

		switch ($new_status) {
			case SQ_STATUS_EDITING:
				// Going to safe edit
				if ($this->status == SQ_STATUS_LIVE) {
					$child_events = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, Array('calendar_event_single', 'calendar_event_recurring'), FALSE);
					file_put_contents($this->data_path.'/.safe_edit_before', serialize($child_events));
					file_put_contents($this->data_path.'/.safe_edit_after', serialize($child_events));
				}//end if
			break;
			case SQ_STATUS_LIVE:
				if ($this->status == SQ_STATUS_EDITING) {
					//we are cancelling safe edit
					$before_safe_edit = Array();
					$after_safe_edit = Array();
					$before_safe_edit_string = file_to_string($this->data_path.'/.safe_edit_before');
					$after_safe_edit_string = file_to_string($this->data_path.'/.safe_edit_after');

					if (!empty($before_safe_edit_string)) {
						$before_safe_edit = @unserialize($before_safe_edit_string);
					}//end if
					if (!empty($after_safe_edit_string)) {
						$after_safe_edit = @unserialize($after_safe_edit_string);
					}//end if

					if (!empty($after_safe_edit)) {
						foreach ($after_safe_edit as $active_link) {
							if (!$this->_compareLink($active_link, $before_safe_edit)) {
								// this link did not exist , move to thrash
								if (isset($active_link['linkid'])) {
									// Drop to a run level to allow the links to be shuffled out without a fuss
									$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
									$this->deleteLink($active_link['linkid']);
									$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
								}// end if
							}// end if
						}// end foreach
					}// end if
					// Coming out of safe edit remove safe edit files
					$this->_cleanupSafeEdit();

				}// end $this->status == SQ_SAFE_EDITING

				if ($this->status == SQ_STATUS_EDITING_APPROVED) {
					// Changes approved
					$this->_cleanupSafeEdit();
				}// end $this->status == SQ_STATUS_EDITING_APPROVED
			break;

		}// end switch

		return parent::processStatusChange($new_status, $update_parents);

	}//end processStatusChange()


	/**
	* Compares the before and after state to check for div/table removal
	*
	* @param array          $current_link   The info about the current link
	* @param array          $before_links   An array of all the before links
	*
	* @return boolean
	* @access private
	*/
	function _compareLink($current_link, $before_links)
	{
		if (!isset($current_link['minorid'])) {
			// Not enough information to proceed
			return FALSE;
		}//end if

		//if before_link is empty, that means there were no dependat events before, just return true
		if (empty($before_links)){
			return FALSE;
		}

		foreach ($before_links as $before_link) {
			if (isset($before_link['minorid'])) {
				if ($before_link['minorid'] == $current_link['minorid']) {
					// It exists!
					return TRUE;
				}//end if
			}//end if
		}//end foreach

		// Didn't find one, report that...
		return FALSE;

	}//end _compareLink()


	/**
	* Cleaning up files after coming out of Safe Edit
	*
	* @return void
	* @access private
	*/
	function _cleanupSafeEdit()
	{
		if (file_exists($this->data_path.'/.safe_edit_after')) {
			unlink($this->data_path.'/.safe_edit_after');
		}//end if
		if (file_exists($this->data_path.'/.safe_edit_before')) {
			unlink($this->data_path.'/.safe_edit_before');
		}//end if

	}//end _cleanupSafeEdit()


}//end class

?>
