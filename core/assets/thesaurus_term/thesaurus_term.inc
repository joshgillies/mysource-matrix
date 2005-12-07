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
* $Id: thesaurus_term.inc,v 1.5 2005/12/07 00:48:34 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Thesaurus_Term
*
* Purpose
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thesaurus_Term extends Asset
{

	/**
	* @var object	$_thesaurus	Reference to the parent thesaurus
	*/
	var $_thesaurus;

	/**
	* @var int	$_termid	Id of this term
	*/
	var $_termid;


	function Thesaurus_Term($asset_id=0)
	{
		if (empty($asset_id)) {
			$this->_loadVars();
			return;
		}
		if (strpos($asset_id, ':') === false) return;

		list($thesaurus_id, $termid) = explode(':', $asset_id);

		$this->_thesaurus =& $GLOBALS['SQ_SYSTEM']->am->getAsset($thesaurus_id);

		$term = $this->_thesaurus->getTermById($termid);
		if (is_null($term)) return;


		$this->id = $asset_id;
		$this->_termid = $termid;
		$this->_loadVars();

		$this->name = $term['term'];
		$this->short_name = $this->name;
		$this->status = $this->_thesaurus->status;

		$this->setAttrValue('name', $term['term']);

	}//end constructor


	/**
	* Create a new asset
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <PRE>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </PRE>
	*
	* @return mixed string|boolean
	* @access public
	*/
	function create(&$link)
	{

		$this->name = $this->attr('name');
		if (empty($this->name)) return false;

		$majorid = null;

		if (is_a($link['asset'], 'thesaurus')) {
			$this->_thesaurus =& $link['asset'];
		} else if (is_a($link['asset'], 'thesaurus_term')) {
			$this->_thesaurus =& $link['asset']->_thesaurus;
			$majorid = $link['asset']->_termid;
		} else {
			trigger_localised_error('CORE0236', E_USER_WARNING, $link['asset']->type());
			return false;
		}

		// see if a term with this name already exists under this parent with this particular relation
		$term = $this->attr('name');
		$relation = $link['value'];

		$created = true;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// check if this term already exists, if it does, then we just link
		$termid = $this->_thesaurus->getTermIdByName($term);
		if (is_null($termid)) {
			$termid = $this->_thesaurus->_addTerm($term);
			if (is_null($termid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error('Unknown error. Cannot add.');
				return false;
			}
			$created = true;
		}


		$this->id = $this->_thesaurus->id.':'.$termid;
		$this->_termid = $termid;

		$existing_link = $this->_thesaurus->getTermLinkByTermids($termid, $majorid, $relation);
		if (!empty($existing_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return null;
		}

		$linkid = $this->_thesaurus->_linkTerms($termid, $majorid, $relation, array_get_index($link,'sort_order'));
		if (is_null($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_localised_error('CORE0237', E_USER_WARNING);
			return false;
		}


		$this->_thesaurus->markContentsChanged();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		if ($created) {
			$em->broadcastEvent($this, 'AssetCreate', Array('name','assetid'));
		}

		return true;

	}//end create()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$links = Array();

		if (isset($this->_termid)) {
			$links = $this->_thesaurus->_getShadowAssetMapLinks($this->_termid);
		}

		return $links;

	}//end getAssetMapLinks()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = true|false
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(
									'thesaurus_term'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> false,
														   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Save the attributes to the DB
	*
	* @return void
	* @access public
	*/
	function saveAttributes()
	{
		if (empty($this->_thesaurus)) return true;

		$name = $this->attr('name');

		$success = $this->_thesaurus->updateTerm($this->_termid, $name);

		if ($success) {
			$this->_thesaurus->saveAttributes();

			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetUpdate', Array('name'));
		}
		return $success;

	}//end saveAttributes()


	/**
	* Create a user-asset link (NOT the same as an asset-asset link)*
	*
	* @param object	$major	Parent asset
	* @param object	$minor	Child asset
	*
	* @return void
	* @access public
	*/
	function createAssetLink($major, $minor)
	{
	}//end createAssetLink()


	/**
	* Returns TRUE if the passed minor asset can be move-linked to this asset, or a string with the error msg
	*
	* By default, use the same tests as being newly linked with a non-exclusive link
	* (as all moved links are non-exclusive by moveLink() specification) - that is,
	* call canCreateLink().
	* However this may not be appropriate for moves in some circumstances (cf. Sites
	* within a Site Network) so this can be used to provide a different check for them.
	* Remember that this is called for the asset we are moving TO - as it would
	* have with canCreateLink().
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param object	&$old_major	the major asset that we are being moved from
	* @param string	$link_type	the type of link this is
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canMoveLink(&$minor, &$old_major, $link_type)
	{
		return $this->canCreateLink($minor, $link_type, 0);

	}//end canMoveLink()


	/**
	* Returns TRUE if the supplied minor can be linked in the specified way
	*
	* @param object	&$minor		the minor asset in the proposed link
	* @param string	$link_type	the type of link that is proposed
	* @param int	$exclusive	(0|1) whether this will be the only significant link
	*							with $minor as the minor asset
	*
	* @return boolean
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		return true;

	}//end canCreateLink()


}//end class
?>