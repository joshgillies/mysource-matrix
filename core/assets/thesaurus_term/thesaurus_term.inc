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
* $Id: thesaurus_term.inc,v 1.24 2013/09/26 08:46:51 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Thesaurus_Term
*
* Purpose
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.24 $
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
		if (strpos($asset_id, ':') === FALSE) return;

		list($thesaurus_id, $termid) = explode(':', $asset_id);

		$this->_thesaurus = $GLOBALS['SQ_SYSTEM']->am->getAsset($thesaurus_id);

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
	* <pre>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </pre>
	*
	* @return mixed string|boolean
	* @access public
	*/
	function create(&$link)
	{
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$this->name = $this->attr('name');
		if (empty($this->name)) return FALSE;

		$majorid = NULL;

		if ($link['asset'] instanceof Thesaurus) {
			$this->_thesaurus =& $link['asset'];
		} else if ($link['asset'] instanceof Thesaurus_Term) {
			$this->_thesaurus =& $link['asset']->_thesaurus;
			$majorid = $link['asset']->_termid;
		} else {
			$type = method_exists($link['asset'], 'type') ? $link['asset']->type() : '';
			trigger_localised_error('CORE0236', sprintf(translate('Cannot create new Thesaurus Term under an asset of type "%1$s"'), $type), E_USER_WARNING)
			return FALSE;
		}

		// see if a term with this name already exists under this parent with this particular relation
		$term = $this->attr('name');
		$relation = isset($this->_tmp['relation']) ? $this->_tmp['relation'] : $link['value'];

		$relation = trim($relation);

		$created = FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// check if this term already exists, if it does, then we just link
		$termid = $this->_thesaurus->getTermIdByName($term);
		if (is_null($termid)) {
			$termid = $this->_thesaurus->_addTerm($term);
			if (is_null($termid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				trigger_error('Unknown error. Cannot add.');
				return FALSE;
			}
			$created = TRUE;
		}

		$this->id = $this->_thesaurus->id.':'.$termid;
		$this->_termid = $termid;

		// if we're adding a top level term it's a stray term - not linked to anything
		if (is_null($majorid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			if ($created) {
				$em->broadcastEvent($this, 'AssetCreate', Array('name','assetid'));
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_created', $this);
			}
			$em->broadcastEvent($this->_thesaurus, 'CreateLink', Array());
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $this->_thesaurus->id.':atoz:'.$termid;
		}

		$existing_link = $this->_thesaurus->getTermLinkByTermids($termid, $majorid, $relation);
		if (!empty($existing_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $this->_thesaurus->id.':'.$existing_link['linkid'];
		}

		$linkid = $this->_thesaurus->linkTerms($termid, $majorid, $relation);
		if (is_null($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			trigger_localised_error('CORE0237', translate('This term cannot be added because it already exists.'), E_USER_WARNING)
			return FALSE;
		}

		$this->_thesaurus->markContentsChanged();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		if ($created) {
			$em->broadcastEvent($this, 'AssetCreate', Array('name','assetid'));
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_created', $this);
		}

		return $this->_thesaurus->id.':'.$linkid;

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
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'thesaurus_term'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
														   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'thesaurus_term'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
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
		if (empty($this->_thesaurus)) return TRUE;

		$name = $this->attr('name');

		// If the name is updated, we are going to update the name of this object so when it broadcasts an
		// even, the new name is used instead. In particular it is
		if ($this->name != $this->attr('name'))	{
			$this->name = $name;
		}//end if

		$success = $this->_thesaurus->updateTerm($this->_termid, $name);//, $notes);

		if ($success) {
			$this->_thesaurus->saveAttributes();

			if (!empty($this->_tmp['vars_set'])) {
				// prepare to broadcast the attributes_changed
				foreach ($this->_tmp['vars_set'] as $attr => $details) {
					$changed_vars[$attr] = Array(
											'type'		=> $this->vars[$attr]['type'],
											'old_value'	=> $details['old_value'],
											'new_value'	=> $this->attr($attr),
										   );
				}
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_attributes_changed', $this, $changed_vars);
			}

			unset($this->_tmp['vars_set']);
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetUpdate', Array('name'));
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_attributes_saved', $this);
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
		$create_link = FALSE;

		if (($minor instanceof Thesaurus_Term) && ($exclusive == 0)) {
			$create_link = (($link_type == SQ_LINK_TYPE_1) || ($link_type == SQ_LINK_TYPE_2));
		}

		return $create_link;

	}//end canCreateLink()


	/**
	* Returns TRUE if the supplied minor can be linked in the specified way
	*
	* @param string	$name	note name
	* @param string	$note	note
	*
	* @return boolean
	* @access public
	*/
	function addTermNote($name, $note)
	{
		$this->_thesaurus->addTermNote($this->_termid, $name, $note);

	}//end addTermNote()


	/**
	* Delete All Term Notes
	*
	* @return void
	* @access public
	*/
	function deleteTermNotes()
	{
		$this->_thesaurus->deleteTermNotes($this->_termid);

	}//end deleteTermNotes()


	/**
	* Get All Term Notes
	*
	* @return Array
	* @access public
	*/
	function getTermNotes()
	{
		return $this->_thesaurus->getTermNotes($this->_termid);

	}//end getTermNotes()

}//end class
?>
