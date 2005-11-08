<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: thesaurus_term.inc,v 1.2 2005/11/08 00:59:26 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Thesaurus_Term
*
* Purpose
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Thesaurus_Term extends Asset
{

	/**
	* @var object	$_thesaurus	Reference to the parent thesaurus
	*/
	var $_thesaurus;

	/**
	* @var string	$_parent_term	Name of the parent term, null for top level
	*/
	var $_parent_term;

	/**
	* @var int	$_term_linkid	Link id this term was found by
	*/
	var $_term_linkid;


	function Thesaurus_Term($asset_id=0)
	{
		if (empty($asset_id)) {
			$this->_loadVars();
			return;
		}
		if (strpos($asset_id, ':') === false) return;


		list($thesaurus_id, $term_linkid) = explode(':', $asset_id);

		$this->_thesaurus =& $GLOBALS['SQ_SYSTEM']->am->getAsset($thesaurus_id);

		$term = $this->_thesaurus->getTermById($term_linkid);
		if (is_null($term)) return;


		$this->id = $asset_id;
		$this->_loadVars();

		$this->name = $term['minor'];
		$this->short_name = $this->name;
		$this->_thesaurus_id = $thesaurus_id;
		$this->_term_linkid = $term_linkid;
		$this->_parent_term = $term['major'];
		$this->status = $this->_thesaurus->status;

		$this->setAttrValue('name', $term['minor']);
		$this->setAttrValue('relation', $term['relation']);

	}//end constructor


	/**
	* Create a new asset
	*
	* @param array	$link	link data
	*
	* @return mixed string|boolean
	* @access public
	*/
	function create($link)
	{
		$this->name = $this->attr('name');
		if (empty($this->name)) return false;


		$shadow_parent = null;
		$parent_term = null;

		if (is_a($link['asset'], 'thesaurus')) {
			$this->_thesaurus_id = $link['asset']->id;
		} else if (is_a($link['asset'], 'thesaurus_term')) {
			$this->_thesaurus_id = $link['asset']->_thesaurus_id;
			$shadow_parent = $link['asset']->id;
			$parent_term = $link['asset']->name;
		}

		if (empty($this->_thesaurus_id)) {
			trigger_localised_error('LEX0016', E_USER_WARNING, $link['asset']->type());
			return false;
		}

		$this->_thesaurus =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_thesaurus_id);

		// see if a term with this name already exists under this parent with this particular relation
		$term = $this->attr('name');
		$relation = $this->attr('relation');

		$assetid = $this->_thesaurus->addTerm($term, $parent_term, $relation);
		if (is_null($assetid)) {
			trigger_localised_error('LEX0017', E_USER_WARNING);
			return false;
		}

		$this->id = $this->_thesaurus_id.':'.$assetid;
		$this->_term_linkid = $assetid;

		$this->_thesaurus->markContentsChanged();

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($link['asset'], 'CreateLink', Array('linkid' => ''));

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

		if (isset($this->_term_linkid)) {
			$links = $this->_thesaurus->_getShadowAssetMapLinks($this->_term_linkid);
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
		$relation = $this->attr('relation');

		$success = $this->_thesaurus->updateTerm($this->_term_linkid, $name, $relation);

		if ($success) {
			$this->_thesaurus->saveAttributes();

			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetUpdate', Array('name' => $name));
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