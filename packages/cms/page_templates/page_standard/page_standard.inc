<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: page_standard.inc,v 1.29 2003/11/21 05:29:25 gsherwood Exp $
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
	* Creates a link between this asset and the passed asset, of a certain type
	*
	* @return int linkid of the created link
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

	}//end createLink()


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

			// get the bodycopy attached to us
			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
			$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link[0]['minorid'], 'bodycopy');
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
		// get the bodycopy attached to us
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, null, 'major', '1');
		if (empty($bodycopy_data)) return;

		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['minor_type_code']);
		$bodycopy->printBody();

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
		// get the bodycopy attached to us
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, null, 'major', '1');
		if (empty($bodycopy_data)) return;

		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['minor_type_code']);
		if (!is_null($bodycopy)) {
			$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('assetid', $bodycopy->id);
			$GLOBALS['SQ_SYSTEM']->backend->out->addHiddenField('backend_assetid', $bodycopy->id);
			$bodycopy->printLimbo();
		}

	}//end printLimbo()


	/**
	* Retrieves a list of key data from this asset (used by the indexing service)
	*
	* Subclasses override this to give the content they want to make searchable.
	*
	* @param mixed	$component	If it is not false, then it is assumed to be the name of a component
	*							of this asset (defined and controlled by subclasses).
	* @return array An array of all the keydata for this asset. Keydata is of the form
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
	function &getKeyData($component = false)
	{
		$keyData = parent::getKeyData($component);

		$weightings = $this->getWeightings();

		if (!$component || $component == 'contents') {
			// get the bodycopy 

			$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, null, 'major', '1');
			if (empty($bodycopy_link)) return;

			$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

			ob_start();
			$bodycopy->printBody();
			$contents = ob_get_contents();
			ob_end_clean();
			
			// preprocess
			$contents = strtolower(strip_tags(html_entity_decode($contents)));
			$word_counts = get_word_counts($contents);
			foreach ($word_counts as $word => $count) {
				$this->buildKeyData($keyData, $word, 'text', 'contents', $count * $weightings['contents']['weighting']);
			}
		}
		return $keyData;

	}// end getKeyData()


}//end class

?>