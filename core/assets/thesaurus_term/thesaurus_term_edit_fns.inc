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
* $Id: thesaurus_term_edit_fns.inc,v 1.1 2005/11/23 00:53:35 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Thesaurus_Term_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thesaurus_Term_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Thesaurus_Term_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		// disable all static screens
		$this->static_screens = Array();

		// enable just the details
		$this->static_screens['details']['name'] = translate('details');
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paints the relation Selector
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintRelationSelector(&$asset, &$o, $prefix)
	{

		if ($this->_tmp['creating']) {
			$parent = array_get_index($_REQUEST, 'parent_assetid');
			if (empty($parent)) return;
			$parent_components = explode(':', $parent);
			$asset->_thesaurus =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_components[0]);
		}

		$write_access = $asset->writeAccess('attributes');
		if ($write_access) {
			$relations = $asset->_thesaurus->getActiveRelations();
			$new_relations = Array();
			foreach ($relations as $relation) {
				$new_relations[$relation] = $relation;
			}

			combo_box($prefix.'_relation', $new_relations, false, $asset->attr('relation'));

			echo translate('thesaurus_or_create_new').': ';
			text_box($prefix.'_relation_new', '', '10');
		} else {
			echo $asset->attr('relation');
		}

		return $write_access;

	}//end paintRelationSelector()


	/**
	* Paints the name of the parent term
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintParentTermName(&$asset, &$o, $prefix)
	{

		if ($this->_tmp['creating']) {
			$parent = array_get_index($_REQUEST, 'parent_assetid');
			if (empty($parent)) return;
			$parent_components = explode(':', $parent);
			$asset->_thesaurus =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_components[0]);
			if (isset($parent_components[1])) {
				$parent = $asset->_thesaurus->getTermById($parent_components[1]);
				$asset->_parent_term = $parent['minor'];
			}
		}

		$parent_term = $asset->_parent_term;
		if (is_null($parent_term)) {
			$parent_term = '-- '.translate('thesaurus_top_level_term').' --';
		}

		echo $parent_term;

	}//end paintParentTermName()


	/**
	* Changes the thesaurus relation attribute if a new relation has been selected
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processRelationSelector(&$asset, &$o, $prefix)
	{
		$relation = array_get_index($_REQUEST, $prefix.'_relation_new');

		if (is_null($relation) || $relation == '') {
			$relation = array_get_index($_REQUEST, $prefix.'_relation');
		}

		$asset->setAttrValue('relation', $relation);
		return true;

	}//end processRelationSelector()


}//end class
?>
