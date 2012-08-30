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
*$Id: asset_cache.inc,v 1.11 2012/08/30 01:09:19 ewang Exp $
*
*/


// If 1, getForgottenItemsReport() will include the callers of the cache
// which forgot to release their reference to the assets
define('SQ_ASSET_CACHE_DEBUG', 0);

/**
* Asset_Cache
*
* Purpose:
*     1.  guarantee that there is maximum one single instance of an
* Asset object in the system, all the client are referencing it.
*     2.  usual cache benefits: prevent excessive access to the
*  disk by holding the assets in memory.
*
* This cache is working with a reference count system, each time an item is
* fetched from the cache its RC is increased. It is decreased by release().
* Once an item's RC drop to 0, it can be removed from the cache.
*
* Items will be automatically removed from the cache when its size exceeds the
* limits. They can also be manually removed by calling remove().
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.11 $
* @package MySource_Matrix
*/
class Asset_Cache extends MySource_Object
{

	/**
	* Contains all the items in the cache and their RC
	* The key is the item id
	*
	* @var Array(int => Array('item' => Object, 'reference_count' -> int))
	*/
	var $_items = Array();

	/**
	* Contains the id of all the released items (RC = 0)
	* The key is the item id, the last item in the array has been released more
	* recently
	*
	* @var Array(int => int)
	*/
	var $_items_released = Array();

	/**
	* The cache size rules
	*
	* Array (
	* 	'max'	 	=> maximum asset allowed,
	* 	'all_items'	=> whether the maximum applies to all the assets
	* 		in the cache or only the released assets
	* )
	*
	* @var Array()
	*/
	var $_size_rules;


	/**
	* Constructor
	*
	* @return Asset_Cache
	* @access public
	*/
	function Asset_Cache()
	{
		$this->setSizeRules();

	}//end constructor


	/**
	* Sets the size rules
	*
	* @param int $size			maximum allowed number of items
	* 							if-1,items will never be automatically removed from the cache
	* @param int $all_items		if true, the rule applies to all the assets in the cache,
	*							otherwise it applies only to the released assets.
	*
	* @return void
	* @access public
	*/
	function setSizeRules($size=-1, $all_items=false)
	{
		$this->_size_rules['max'] = $size;
		$this->_size_rules['all_items'] = $all_items;

	}//end setSizeRules()


	/**
	* Adds an item in the cache
	* Sets the ref count of the item to 1.
	*
	* @param object $item	the item to add
	* @param string $itemid	the item id
	* 						(if	-1, the id is determined by $item->id)
	*
	* @return boolean 		false if the item is already in the cache
	* @access public
	*/
	function add($item, $itemid=-1)
	{
		if ($itemid == -1) {
			// no valid id
			if (!isset($item) || !isset($item->id)) {
				return false;
			}
			$itemid = $item->id;
		}

		// already in the cache
		if ($this->get($itemid)) return false;

		$this->_items[$itemid] = Array('item' => $item, 'reference_count' => 1);

		if (SQ_ASSET_CACHE_DEBUG) {
			$this->_trackCaller($itemid, 1);
		}

		return true;

	}//end add()


	/**
	* Fetch an item from the cache
	* Returns a reference to this item (null if not found)
	*
	* @param string $itemid	the id of the item to get
	*
	* @return object
	* @access public
	*/
	function get($itemid)
	{
		$ret = null;

		if (isset($this->_items[$itemid])) {
			$item_info = &$this->_items[$itemid];

			$ret = &$item_info['item'];

			if ($item_info['reference_count'] == 0) {
				unset($this->_items_released[$itemid]);
			}

			$item_info['reference_count']++;

			if (SQ_ASSET_CACHE_DEBUG) {
				$this->_trackCaller($itemid, 1);
			}
		}

		return $ret;

	}//end get()


	/**
	* Release an item from the cache.
	* Practicaly, it just decreases the RC of the item, if the RC drops to 0 it
	* may be removed from the cache.
	*
	* @param string $itemid	the id of the  item to release
	*
	* @return boolean 	false if the item is not in the cache or its RC = 0
	* @access public
	*/
	function release($itemid)
	{
		if (!isset($this->_items[$itemid])) return false;

		$item_info = &$this->_items[$itemid];

		if ($item_info['reference_count'] <= 0) return false;

		$item_info['reference_count']--;

		if (SQ_ASSET_CACHE_DEBUG) {
			$this->_trackCaller($itemid, -1);
		}

		if ($item_info['reference_count'] == 0) {
			$this->_items_released[$itemid] = $itemid;
			$this->_applySizeRules();
		}

		return true;

	}//end release()


	/**
	* Applies the size rules, if the current cache size exceeds the rules an
	* item will be removed.
	*
	* @return void
	* @access private
	*/
	function _applySizeRules()
	{
		// size -1 disable automatic remove
		if ($this->_size_rules['max'] == -1) return;

		if (count($this->_items_released) && $this->getSize(true)) {
			$current_size = $this->getSize(!$this->_size_rules['all_items']);

			if ($current_size > $this->_size_rules['max']) {
				// rules not respected, remove the oldest item
				reset($this->_items_released);
				$this->remove(current($this->_items_released));
			}
		}

	}//end _applySizeRules()


	/**
	* Remove an item from the cache
	* The item must have a RC = 0
	*
	* @param string $itemid	id of the item to remove
	* @param boolean $force	if true, the asset is removed even if its RC is > 0,
	*						this option shouldn't be used, it is there to remain
	*						compatible with the old code.
	*
	* @return boolean	false if the item is not in the cache or RC > 0
	* @access public
	*/
	function remove($itemid, $force=false)
	{
		if (!$force && ($this->_items[$itemid]['reference_count'] != 0)) {
			return false;
		}

		unset($this->_items[$itemid]);
		unset($this->_items_released[$itemid]);

		return true;

	}//end remove()


	/**
	* Remove all released items from the cache
	*
	* @return void
	* @access public
	*/
	function removeAll()
	{
		$itemsid = array_keys($this->_items_released);
		foreach ($itemsid as $itemid) {
			$this->remove($itemid);
		}

	}//end removeAll()


	/**
	* Returns the number of items in the cache.
	*
	* @param boolean $released	if true only returns the number of released items
	* 							if false returns the total number of items
	*
	* @return int
	* @access public
	*/
	function getSize($released=false)
	{
		return $released ? count($this->_items_released) : count($this->_items);

	}//end getSize()


	/**
	* Returns a information string about all the assets
	* in the cache which Ref Count > 0
	*
	* @return string
	* @access public
	*/
	function getForgottenItemsReport()
	{
		$ret = '';

		ob_start();

		echo "Asset Cache report:\n";

		$item_count = 0;
		foreach ($this->_items as $itemid => $item_info) {
			if ($item_info['reference_count'] > 0) {
				echo 'Asset #'.$itemid.'  ('.get_class($item_info['item']).')  RC='.$item_info['reference_count']."\n";
				if (isset($item_info['callers'])) {
					print_r ($item_info['callers']);
				}
				$item_count++;
			}
		}
		echo $item_count." items \n";

		$ret = ob_get_clean();

		return $ret;

	}//end getForgottenItemsReport()


	/**
	* Add information in the cache about the caller class
	*
	* @param	string	$itemid		id of the item which RC has changed
	* @param	int		$rc_diff	difference in the reference count
	*  								(eg. +1, -1)
	*
	* @return void
	* @access private
	*/
	function _trackCaller($itemid, $rc_diff)
	{
		// 1. get class name
		$bt = debug_backtrace();

		$caller = '???';
		$excluded_classes = Array('asset_manager', 'asset', strtolower(__CLASS__));
		for ($caller_idx = 0; $caller_idx < count($bt); $caller_idx++) {
			$class = (isset($bt[$caller_idx]['class'])) ? $bt[$caller_idx]['class'] : '';
			if (!in_array(strtolower($class), $excluded_classes)) {
				break;
			}
		}
		if ($caller_idx < count($bt)) {
			$caller = (isset($bt[$caller_idx]['class'])) ? $bt[$caller_idx]['class'] : '';
			$caller .= ':'.$bt[$caller_idx]['file'];
		}

		// 2. add info to the cache
		if (!isset($this->_items[$itemid]['callers'])) {
			$this->_items[$itemid]['callers'] = Array();
		}
		if (!isset($this->_items[$itemid]['callers'][$caller])) {
			$this->_items[$itemid]['callers'][$caller] = 0;
		}
		$this->_items[$itemid]['callers'][$caller] += $rc_diff;

	}//end _trackCaller()


	/**
	*  Returns all item ids currently loaded in asset cache
	*
	* @return array
	* @access public
	*/
	function getAssetKeys()
	{
		return array_keys($this->_items);

	}//end getAssetKeys()


}//end class

?>
