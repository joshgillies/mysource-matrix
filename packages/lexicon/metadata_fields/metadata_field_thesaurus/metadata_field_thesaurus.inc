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
* $Id: metadata_field_thesaurus.inc,v 1.2 2004/12/02 03:40:58 arailean Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/metadata/metadata_field/metadata_field.inc';

/**
* Metadata_Field_Select
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Metadata_Field_Thesaurus extends Metadata_Field
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Metadata_Field_Thesaurus($assetid=0)
	{
		$this->Metadata_Field($assetid);

	}//end constructor


	/**
	* Backend Painter
	*
	* @param $o object    bakcend outputter
	*
	* @access public
	* @return void
	*/
	function paintBackend(&$o)
	{
		$prefix = $this->getPrefix();

		$direct = array_get_index($_REQUEST, $prefix.'_direct_connection', false);

		// if a direct connection is requested, pass the control
		// to the direct Communicator, otherwise take normal course of action
		if ($direct) {
			$this->_directCommunicator();
			exit; // need this since Matrix tries to print extras after this section is done
		} else {
			parent::paintBackend(&$o);
		}

	}//end paintBackend()


	/**
	* Direct Communicator
	*
	* Performs the direct communication necessary the edit interface JS to
	* be updated with new content dynamically
	*
	* This function outputs JavaScript and assumes the echo statements to
	* produce a complete response - i.e. no more output needs to be done by matrix
	*
	* @access public
	* @return void
	*/
	function _directCommunicator()
	{
		$prefix = $this->getPrefix();

		$thesaurus = $this->getThesaurusRef();

		$term_name      = array_get_index($_REQUEST, $prefix.'_entity_name');
		$request_info   = array_get_index($_REQUEST, $prefix.'_info', false);
		$request_init   = array_get_index($_REQUEST, $prefix.'_init', false);

		if ($request_init) {
			if (!empty($term_name)) {
				$init_list_full = $thesaurus->searchEntities($term_name);
			}
			elseif ($thesaurus->isRelationEnabled($thesaurus->REL_CATEGORY)) {
				$rel = $thesaurus->getRelationByName($thesaurus->REL_CATEGORY);
				$init_list_full = $thesaurus->getAbsoluteParentsInRelation($rel["id"]);
			}
			if (isset($init_list_full)) {
				foreach ($init_list_full as $child) {
					$init_list[] = $child["name"];
				}
			}
			$out = 'parent.Browser.setNewList(Array(Array("Result"), Array(Array("'.implode('","', $init_list).'"))))';

		} else if (isset($term_name)) {
			$term = $thesaurus->getEntityByName($term_name);
			if (empty($term)) {
				$out = 'alert("Term not found in thesaurus");';
			}

			if ($request_info) { // only requesting info
				$description = $term['description'];
				$out = "parent.Browser.entityInfo(Array('$term_name','Description: $description.'));";
			} else { // requesting the list of related entities
				$child_relations = $thesaurus->getChildRelationsForEntity($term['id']);
				if (!empty($child_relations)) {
					foreach ($child_relations as $relation) {
						$child_e_names = Array();
						$rel_id = $relation['id'];
						$child_relation_names[] = $relation['name'];
						$child_entities = $thesaurus->getChildEntitiesInRelation($rel_id, $term['id']);

						foreach ($child_entities as $child) {
							$child_e_names[] = $child['name'];
						}

						$js_relation_entity_array[] = "Array('".implode("','",$child_e_names)."')";
					}

					$js_relation_array = "Array('".implode("','",$child_relation_names)."')";
					$js_r_entity_array = "Array(".implode(",",$js_relation_entity_array).")";
					$js = "$js_relation_array,$js_r_entity_array";
				} else {
					$js = '';
				}

				$out  = "parent.Browser.setNewList(Array($js))";
			}
		}

		// output
		echo '
			<script type="text/javascript">
			if (parent.Browser) {
				if (typeof(parent.Browser) != "undefined" && parent.Browser != null) {'.$out.'}
			}
			</script>
		';

	}//end _directCommunicator()



	/**
	* Get URL of this asset for direct communication
	*
	* Returns a backend URL of this asset that can service direct connections
	* (ignoring backend frames). This function is used by the JS browser
	*
	* @access public
	* @return void
	*/
	function getCommunicatorUrl()
	{
		return $this->getBackendHref().'&'.$this->getPrefix().'_direct_connection=true&ignore_frames=1';

	}//end getCommunicatorUrl()


	/**
	* Returns a reference to the parent thesaurus asset
	*
	* @access public
	* @return object    thesaurus
	*/
	function &getThesaurusRef()
	{
		$th_id = $this->attr('thesaurus_assetid');
		if ($th_id == -1) {
			return false;

		}

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($th_id);

	}//end getThesaurusRef()


	/**
	* Check if thesaurus is active
	*
	* @access public
	* @return boolean
	*/
	function isThesaurusActive()
	{
		$th = &$this->getThesaurusRef();

		return ($th && $th->isActive());

	}//end isThesaurusActive()


}//end class

?>