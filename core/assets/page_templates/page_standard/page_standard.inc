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
* $Id: page_standard.inc,v 1.38 2013/07/25 23:23:48 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Page_Standard
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.38 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_Standard extends Page
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
	* Perform any additional processing required during the creation of this asset
	*
	* Standard pages create a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link, $args=Array())
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$bodycopy->setAttrValue('name', 'Page Contents');
		if ($bodycopy->create($copy_link, $args)) {
			$child_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_div');
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($child_bodycopy['minorid'], 'attributes');
			return TRUE;
		}
		return FALSE;

	}//end _createAdditional()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param Asset	$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if a bodycopy is linking to us then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Bodycopy) && $dependant != '1') {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	public function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', NULL, '1', '1');
		if (!$GLOBALS['SQ_PURGING_TRASH'] && (!empty($bodycopy_link) && $bodycopy_link[0]['linkid'] == $linkid)) {
			return translate('You can\'t delete the link between the bodycopy and the standard page');

		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_1]['calendar_event_single']    = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_1]['calendar_event_recurring'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_1]['data_record'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['calendar_event_single']    = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['calendar_event_recurring'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 1, 'exclusive' => TRUE);
		$page_links[SQ_LINK_TYPE_2]['data_record'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['calendar_event_single']    = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_NOTICE]['calendar_event_recurring'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Prints out the backend interface for the asset
	*
	* @param Backend_Outputter	$o	Backend_Outputter
	*
	* @return void
	* @access public
	*/
	public function paintBackend(Backend_Outputter $o)
	{
		if (isset($_GET['sq_from_frontend'])) {

			$bodycopy = $this->getBodycopy();
			if (is_null($bodycopy)) {
				trigger_localised_error('CORE0016', $this->name.'"', E_USER_ERROR);
				return FALSE;
			}
			$o->addFormActionGetVar('assetid', $bodycopy->id);

			// fudge the navigation history a little
			if (!isset($_SESSION['sq_nav_history'])) {
				$_SESSION['sq_nav_history'] = Array();
				$nav_pos = 0;
			} else {
				$nav_pos = count($_SESSION['sq_nav_history']) - 1;
			}
			$_SESSION['sq_nav_history'][$nav_pos] = Array('assetid' => $this->id, 'screen' => 'details');

			$bodycopy->paintBackend($o);

		} else {

			parent::paintBackend($o);
		}

	}//end paintBackend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		if ($contextid === 0) {
			$cache_key = 'default';
		} else {
			$cache_key = 'default.'.$contextid;
		}

		// evaluate conditions to show conditional divs and replace conditional keywords
		$conditions = $this->getConditions();
		if(!empty($conditions)) {
		    $GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'evaluate conditions');
		    $this->evaluateConditions();
		    // the cache key must include the condition eval results, so when condition changed, cache object will change
		    if(isset($GLOBALS['SQ_CONDITION']))
			$cache_key .= serialize($GLOBALS['SQ_CONDITION']);
		    $GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'evaluate conditions');
		}


		// start performance timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'print content');

		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$cached_contents = $cm->loadFromCache($this->id, $this->type(), $cache_key);


		// if the cache is empty, go ahead and regenerate a new version
		if ($cached_contents === FALSE) {
			$bodycopy = $this->getBodycopy();
			if (is_null($bodycopy)) return;
			ob_start();
				// set the keyword replacements
				$keywords = $bodycopy->getKeywords();
				$replacements = Array();
				foreach ($keywords as $word) {
					$replacements[$word] = $this->getKeywordReplacement($word);
				}

				$bodycopy->setKeywordReplacements($replacements);

				$bodycopy->printBody();

				$cm->saveToCache($this->id, $this->type(), $cache_key, ob_get_contents());
			ob_end_flush();
		} else {
			echo $cached_contents;
		}

		// end performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'print content');

	}//end printBody()


	/**
	* Initialise the limbo (simple-edit) interface by painting to the limbo outputter
	*
	* Standard page puts its bodycopy up for editing instead of itself
	*
	* @return void
	* @access public
	*/
	public function initLimbo()
	{
		$lm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
		$screen = array_get_index($_REQUEST, 'asset_ei_screen', 'details');

		// Look for a layout for the details screen
		$layout_pattern = 'layout::'.$this->type().'::details';
		$details_layout = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL(current_url(FALSE, TRUE), $layout_pattern, TRUE);

		if (!empty($details_layout) && ($details_layout > 0)) {
			parent::initLimbo();
		} else if ($lm->hasLayout($this, $screen)) {
			// Otherwise lookup layout manager
			parent::initLimbo();
		} else {
			$bodycopy = $this->getBodycopy();
			if (!is_null($bodycopy)) {
				$edit_interface = $bodycopy->getEI();
				$available_screens = array_keys(array_merge($edit_interface->getScreens(), $edit_interface->_edit_fns->static_screens));
				if (in_array($screen, $available_screens)) {
					$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('assetid', $bodycopy->id);
					$GLOBALS['SQ_SYSTEM']->backend->out->addHiddenField('backend_assetid', $bodycopy->id);
					$bodycopy->initLimbo();
				} else {
					parent::initLimbo();
				}
			}
		}

	}//end initLimbo()


	/**
	* Returns the raw content of this asset
	*
	* @param boolean $empty_content If TRUE, empty content will be returned
	*
	* @return string
	* @access public
	*/
	public function getContent($empty_content=TRUE)
	{
		$bodycopy = $this->getBodycopy();
		if (is_null($bodycopy)) return '';
		$contents = $bodycopy->getRawBodycopyContent();

		return $contents;

	}//end getContent()


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
	* Return any locks that are needed for editing, based on a set of custom keywords
	*
	* @param array	$keywords	the keywords to govern what locks are required
	*
	* @return array
	* @access public
	*/
	public function getEditingLocks(Array $keywords)
	{
		$locks = Array();
		if (!is_array($keywords) || empty($keywords)) {
			return Array();
		}
		if (!in_array('contents', $keywords)) return Array();
		return Array('content');

	}//end getEditingLocks()


	/**
	* Return the bodycopy linked to this page
	*
	* @return Bodycopy
	* @access public
	*/
	public function getBodycopy()
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, '', 'major');
		if (empty($link)) return $null;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $bodycopy;

	}//end getBodycopy()


	/**
	* Get replacements for keywords that are specific to this asset
	*
	* @param array	$keywords	the keywords to get replacements for
	*
	* @return array
	* @access public
	* @see Asset::getCustomKeywordReplacements()
	*/
	public function getCustomKeywordReplacements($keywords, $invoke_backend = FALSE)
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
	public function processCustomKeywords(Array $keywords)
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
	*
	* Call this function when the asset has been changed in some way (or its containers), so we should update the condition evaluation array of it
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=TRUE)
	{
		if (!parent::_updated($update_parents)) return FALSE;

		$bodycopy = $this->getBodycopy();
		if(empty($bodycopy)) return TRUE;

		return ($this->extractConditionsFromBodycopy(Array($bodycopy->id)));
	}//end _updated()




	/**
	* Add condition keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();

		// add common asset keywords to standard page
		parent::onRequestKeywords($broadcaster, $vars);

		$keywords = $this->getAvailableConditionKeywords();
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()





}//end class

?>
