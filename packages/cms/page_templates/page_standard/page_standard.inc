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
* $Id: page_standard.inc,v 1.31.2.1 2004/02/18 12:30:09 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Page_Standard
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Standard extends Page
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Standard($assetid=0)
	{
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
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		$bodycopy->setAttrValue('name', 'Page Contents');
		if (!$bodycopy->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


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
		// if a bodycopy is linking to us then we need to make it a dependant link
		if ($side_of_link == 'major' && is_a($asset, 'bodycopy') && $dependant != '1') {
			$dependant = '1';
			return true;
		}

		return false;

	}//end prepareLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the standard page';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


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
	* @return void
	* @access public
	*/
	function paintBackend(&$o)
	{
		if (isset($_GET['sq_from_frontend'])) {
			
			$bodycopy = &$this->getBodycopy();
			if (is_null($bodycopy)) {
				trigger_error('Could not locate the bodycopy for standard page "'.$this->name.'"', E_USER_ERROR);
				return false;
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
	function printBody()
	{
		$bodycopy = &$this->getBodycopy();
		if (!is_null($bodycopy)) $bodycopy->printBody();

	}//end printBody()


	/**
	* Called by the design to print the body of this asset when in limbo
	*
	* Standard page puts its bodycopy up for editing instead of itself
	*
	* @return void
	* @access public
	*/
	function printLimbo()
	{
		$lm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
		
		if ($lm->hasLayout($this->type(), 'details')) {
			parent::printLimbo();
		} else {
			$bodycopy = &$this->getBodycopy();
			if (!is_null($bodycopy)) {
				$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('assetid', $bodycopy->id);
				$GLOBALS['SQ_SYSTEM']->backend->out->addHiddenField('backend_assetid', $bodycopy->id);
				$bodycopy->printLimbo();
			}
		}

	}//end printLimbo()


	/**
	* Retrieves a list of key data from this asset (used by the indexing service)
	*
	* Subclasses override this to give the content they want to make searchable.
	*
	* @param mixed	$component	If it is not false, then it is assumed to be the name of a component
	*							of this asset (defined and controlled by subclasses).
	* @return Array An array of all the keydata for this asset. Keydata is of the form
	* <PRE>
	* Array (
	* 	Array (
	* 		'value' => ...,
	* 		'type' => ..., (text, date, number, etc.)
	* 		'component' => ..., (whatever you want, just as long as this asset knows how to deal with it if it needs to)
	* 	), ...
	* )
	* </PRE>
	* @access public
	*/
	function getKeyData($component=null)
	{
		$keydata = parent::getKeyData($component);
		$weightings = $this->getWeightings();

		if (is_null($component) || $component == 'contents') {
			// get the bodycopy
			$bodycopy = &$this->getBodycopy();
			if (is_null($bodycopy)) return $keydata;

			ob_start();
				$bodycopy->printBody();
				$contents = ob_get_contents();
			ob_end_clean();
			
			// preprocess
			$contents = strtolower(strip_tags(html_entity_decode($contents)));
			$comp_name = 'attr:'.$this->type().':contents';
			foreach (get_word_counts($contents) as $word => $count) {
				$this->buildKeyData($keydata, $word, 'text', $comp_name, $count * $weightings['contents']['weighting']);
			}
		}
		return $keydata;

	}//end getKeyData()


	/**
	* Acquires any locks that are needed for editing, based on a set of custom keywords
	*
	* @param Array() $keywords the keywords to govern what locks are required
	*
	* @return boolean
	* @access public
	*/
	function acquireEditingLocks($keywords)
	{
		if (!is_array($keywords) || empty($keywords)) return false;
		$acquire = false;
		foreach ($keywords as $keyword) {
			switch ($keyword) {
				case 'contents' :
					$bodycopy = &$this->getBodycopy();
					if (is_null($bodycopy)) continue;
					// we need the 'links' lock to edit the bodycopy
					$vars = Array('assetid' => $bodycopy->id, 'lock_type' => 'content', 'forceably_acquire' => false);
					$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$hh->queueHipo('hipo_job_acquire_lock', $vars);
					$acquire = true;
				break;
			}//end switch
		}//end foreach 

		return $acquire;

	}//end acquireEditingLocks()


	/**
	* Returns the bodycopy linked to this page
	*
	* @return &object Asset
	* @access public
	*/
	function &getBodycopy()
	{
		$null = null;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, '', 'major');
		if (empty($link)) return $null;
		
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
	
	}//end getBodycopy()


	/**
	* Gets replacements for keywords that are specific to this asset
	*
	* @param Array $keywords the keywords to get replacements for
	*
	* @return Array()
	* @access public
	*
	* @see Asset::getCustomKeywordReplacements()
	*/
	function getCustomKeywordReplacements($keywords)
	{
		if (!is_array($keywords) || empty($keywords)) return Array();

		$replacements = Array();
		foreach ($keywords as $keyword) {
			switch ($keyword) {
				case 'contents' :
					$bodycopy = &$this->getBodycopy();
					if (is_null($bodycopy)) continue;
					$edit = &$bodycopy->getEditFns();
					ob_start();
					$edit->paintBodycopy($bodycopy, $GLOBALS['SQ_SYSTEM']->backend->out, $bodycopy->getPrefix());
					$replacements[$keyword] = ob_get_contents();
					ob_end_clean();
				break;

				default:
					$replacements[$keyword] = '';
				break;
			}//end switch
		}//end foreach

		return $replacements;

	}//end getCustomKeywordReplacements()


	/**
	* processes anything that was previously printed for a custom keyword
	*
	* @param Array $keywords the keywords that were originally printed
	*
	* @return boolean
	* @access public
	*
	* @see Asset::processCustomKeywords()
	*/
	function processCustomKeywords($keywords)
	{
		if (!is_array($keywords) || empty($keywords)) return Array();

		foreach ($keywords as $keyword) {
			switch ($keyword) {
				case 'contents' :
					$bodycopy = &$this->getBodycopy();
					if (is_null($bodycopy)) continue;
					$edit = &$bodycopy->getEditFns();
					$edit->processBodycopy($bodycopy, $GLOBALS['SQ_SYSTEM']->backend->out, $bodycopy->getPrefix());
				break;
			}//end switch
		}//end foreach
		
		return true;

	}//end processCustomKeywordReplacements()


}//end class
?>