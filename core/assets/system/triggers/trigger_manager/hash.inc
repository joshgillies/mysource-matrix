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
* $Id: hash.inc,v 1.11.28.1 2012/03/12 04:02:47 mhaidar Exp $
*
*/

/**
*
* hash.inc
*
* To serve as a container for the state hash used by the Trigger system
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.11.28.1 $
* @package MySource_Matrix
*/
class Hash
{

	/**
	* @var array	$hash	an array of all the components of this hash object
	*/
	var $hash;
	
	/**
	* @var array	$tree_propagation	map of tree id to its propagation setting
	* @see Hash::setLinkTree()
	*/
	var $tree_propagation;

	/**
		// Allowed Hash Indices

		event
		assetid
		asset_type
		treeid
		tree_propagate
		triggerid
		parentid
		parent_type
		childid
		child_type
	*/


	/**
	* Constructor
	*/
	function Hash()
	{
		$this->hash = Array();
		$this->tree_propagation = Array();

	}//end constructor


	/**
	* Provides a status of whether this hash object is empty or not
	*
	* @return boolean
	* @access public
	*/
	function isEmpty()
	{
		return empty($this->hash);

	}//end isEmpty()


	/**
	* Internal function for setting the value of a hash field
	*
	* @param string	$name	variable to modify
	* @param mixed	$value	value that the variable will be set to
	*
	* @return void
	* @access private
	*/
	function _set($name, $value)
	{
		if (!isset($this->hash[$name])) {
			$this->hash[$name] = Array();
		}

		if (!is_array($value)) $value = Array($value);

		foreach ($value as $one_value) {
			if (!in_array($one_value, $this->hash[$name])) {
				$this->hash[$name][] = $one_value;
			}
		}

	}//end _set()


	/**
	* Sets the Event field
	*
	* @param string	$event	the name of the event that was fired
	*
	* @return void
	* @access public
	*/
	function setEvent($event)
	{
		$this->_set('event',$event);

	}//end setEvent()


	/**
	* Sets the AssetID field
	*
	* @param string	$assetid	the id of the asset
	*
	* @return void
	* @access public
	*/
	function setAssetId($assetid)
	{
		$this->_set('assetid',$assetid);

	}//end setAssetId()


	/**
	* Sets the Asset Type field
	*
	* @param string		$asset_type		the type of the asset
	* @param boolean	$descendants	whether the hash will include the type's descendants, or just the passed asset type
	*
	* @return void
	* @access public
	*/
	function setAssetType($asset_type, $descendants=FALSE)
	{
		if (!empty($asset_type) && $GLOBALS['SQ_SYSTEM']->am->installed($asset_type)) {

			if ($descendants) {

				// Grab the list of types, put into the 'asset_type' field
				$type_descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($asset_type, TRUE);
				if (!empty($type_descendants)) {
					$this->_set('asset_type', $type_descendants);
				}

			} else {
				$this->_set('asset_type', $asset_type);
			}

		}

	}//end setAssetType()


	/**
	* Sets the Asset Type field
	*
	* @param string	$tree_id		the raw link tree id of the asset (see asset_manager::getAssetTreeids)
	* @param string	$propagation	'01' or '00'. Determines whether the we are checking for assets underneath a tree node, or just for an individual asset
	*
	* @return void
	* @access public
	*/
	function setLinkTree($tree_id, $propagation)
	{
		$this->_set('treeid',$tree_id);
		
		if (!is_array($tree_id)) $tree_id = Array($tree_id);
		foreach ($tree_id as $id){
			$this->tree_propagation[$id] = $propagation;
		}

	}//end setLinkTree()


	/**
	* Sets the Trigger ID field
	*
	* @param string	$trigger_id	the ID of the trigger
	*
	* @return void
	* @access public
	*/
	function setTriggerId($trigger_id)
	{
		$this->_set('triggerid',$trigger_id);

	}//end setTriggerId()


	/**
	* Sets either the parent or child ID field of the Linked asset
	*
	* @param string		$linked_id	the ID of the linked asset
	* @param boolean	$is_major	whether the linked asset is a parent or a child
	*
	* @return void
	* @access public
	*/
	function setLinkedId($linked_id, $is_major)
	{
		if (!$is_major) {
			$this->_set('parentid',$linked_id);
		} else {
			$this->_set('childid',$linked_id);
		}

	}//end setLinkedId()


	/**
	* Sets either the parent or child Type field of the Linked asset
	*
	* @param string		$linked_type	the type_code of the linked asset
	* @param boolean	$is_major		whether the linked asset is a parent or a child
	*
	* @return void
	* @access public
	*/
	function setLinkedType($linked_type, $is_major)
	{
		if (!$is_major) {
			$this->_set('parent_type',$linked_type);
		} else {
			$this->_set('child_type',$linked_type);
		}

	}//end setLinkedType()


	/**
	* Takes the hash, checks it for empty fields filling in the blanks where necessary,
	* and creates all possible combinations of its elements
	*
	* @return void
	* @access public
	*/
	function getHashCombinations()
	{
		$result = $this->hash;

		// make sure every field has data, and that no fields are null
		foreach ($result as $id => $data) {
			if (empty($result[$id])) $result[$id] = '-1';
		}


		$indicies = Array(
						'event',
						'assetid',
						'asset_type',
						'treeid',
						'tree_propagate',
						'triggerid',
						'parentid',
						'parent_type',
						'childid',
						'child_type',
					);
		foreach ($indicies as $index) {
			if (!isset($result[$index])) $result[$index] = '-1';
		}
		
		$combinations = $this->_createCombinations($result);
		
		// Bug #5612, we should not create combinations of the 'tree_propagate' index, so manually populate.
		if ($result['treeid'] != '-1'){
			foreach ($combinations as &$data){
				$data['tree_propagate'] = $this->tree_propagation[$data['treeid']];
			}	
		}

		return $combinations;

	}//end getHashCombinations()


	/**
	* Given an array of elements of various depth, creates all possible combinations
	* of the elements, preserving the index
	* Given:
	* <code>
	*   $source = Array(1=>Array('a','b'), 2=>'c', 3=>Array('d'));
	*   $result = Array(Array(1=>'a',2=>'c',3=>'d'), Array(1=>'b',2=>'c',3=>'d'));
	* </code>
	* This function can be called statically
	* returns Array(mixed)  $state_hash     The flattened state_hash
	*
	* NOTE: Can only create combinations for arrays two levels deep
	*
	* @param array	$source	The hash to flatten
	*
	* @return array
	* @access private
	*/
	function _createCombinations($source)
	{
		$result = Array();

		// step through each item in the source array
		// and make combinations of it with the other items,
		// preserving the position (index)
		// so the item with index 'a' will always be at index 'a'
		// if given an array of type Array(1=>'a',2=>'b',3=>'c'), the result will be the same array
		// if given Array(1=>Array('a','b'), 2=>Array('c')), the result is Array(Array(1=>'a',2=>'c'),Array(1=>'b',2=>'c'))
		// in short, this function creates a flat representation of the deep array, which can
		// easily be converted back to the deep version if needed, i.e. no information is lost

		// let's roll this one
		foreach ($source as $source_key => $source_item) {
			reset($result);
			$new_result = Array();

			if (!is_array($source_item)) {
				$source_item = Array($source_item);
			}

			if (empty($result)) {
				foreach ($source_item as $one_item) {
					$new_result[] = Array($source_key => $one_item);
				}
			} else {
				foreach ($result as $res) {
					foreach ($source_item as $one_item) {
						$new_res = $res;
						$new_res[$source_key] = $one_item;
						$new_result[] = $new_res;
					}
				}
			}

			$result = $new_result;
		}

		return $result;

	}//end _createCombinations()


}//end class


?>
