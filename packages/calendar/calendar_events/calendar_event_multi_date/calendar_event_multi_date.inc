<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event_multi_date.inc,v 1.7 2008/12/01 22:57:18 akarelia Exp $
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
* @version $Revision: 1.7 $
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
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
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
		$allowed[SQ_LINK_TYPE_2]['folder'] = Array('card' => 'M', 'exclusive' => FALSE);
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
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$cached_contents = $cm->loadFromCache($this, 'default');

		// if the cache is empty, go ahead and regenrate a new version
		if ($cached_contents === FALSE) {
			// get the bodycopy here to avoid caching of any possible error messages
			$bodycopy = $this->getBodycopy();
			ob_start();
				// we only want to cache if we actually have a bodycopy
				if (!is_null($bodycopy)) {
					$bodycopy->printBody();
					$cm->saveToCache($this->id, $this->type(), 'default', ob_get_contents());
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
	* @see Asset::_getName()
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

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
	function getCustomKeywordReplacements($keywords)
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
					ob_start();
						if ($edit->paintBodycopy($bodycopy, $GLOBALS['SQ_SYSTEM']->backend->out, $bodycopy->getPrefix())) {
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


}//end class

?>
