<?php

/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: indexing_manager.inc,v 1.3 2003/10/14 05:29:28 dwong Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Indexing_Manager
*
* Handles requests for indexing keydata from searchable assets, and handles search requests.

*
* @author  Dominic Wong<dwong@squiz.net>
* @version $Revision: 1.3 $ - 1.0
* @package MySource_Matrix
*/
class Indexing_Manager extends MySource_Object 
{
	/** 
	 * Constructor
	 * 
	*/

	function Indexing_Manager() 
	{
		parent::MySource_Object();
	} // end constructor

	/** 
	  * Function called from when an asset processes changes to signify
	  * that the keydata should be flushed and freshly retrieved.
	  * 
	  * @param int $assetid ID of the asset.
	  * @param optional $component The component to which the keydata pertains (e.g. metadata or attribute)
	  * @access public
	  * @returns void
	  * 
	*/
	function changed($assetid, $component = false) 
	{
		if ($assetid <= 0)
			return;

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$this->flushKeyData($assetid, $component);

		$asset =& $am->getAsset($assetid);
		$newKeyData =& $asset->getKeyData($component);

		$this->addKeyData($assetid, $newKeyData);
	} // end changed()

	/**
	* Flushes the keydata for an asset.
	* 
	* @param int $assetid ID of the asset
	* @param optional $component The component to which the keydata pertains (e.g. metadata or attribute)
	* @access public
	* @returns void
	*/
	function flushKeyData($assetid, $component = false)
	{
		$sql = "DELETE FROM " . SQ_TABLE_PREFIX . "asset_indexing 
				WHERE assetid = '" . addslashes($assetid) . "'\n";
		if ($component !== false) {
			$sql .= "AND component = '" . addslashes($component) ."'";
		}
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$db->query($sql);
	} // end flushKeyData()

	/** 
	  * Adds keydata for an asset.
	  * 
	  * @param array ref $keydata An array of the form : 
	  *		array (
	  *			0 => array (
	  *				'value' => ... , 
	  *				'type' => ... , 
	  *				'component' => ...   (optional, defaults to '')
	  *			), 
	  *			... 
	  *		)
	  * @access private
	  * @returns void
	  * 
	*/
	function addKeyData($assetid, &$keyData) 
	{	
		if (!is_array($keyData))
			return;
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$paramData = array();
		foreach($keyData as $i => $datum) {
			# convert to ISO 8601 date format Y-m-d H:i:s if not already
			$datum =& $keyData[$i];
			if ($datum['type'] == 'date') {
				if (!(ereg('[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}', $datum['type']))) {
					$datum['value'] = date("Y-m-d H:i:s", strtotime($datum['value']));
				}
			} else if ($datum['type'] == 'text') {
				$datum['value'] = strtolower($datum['value']);
			}

			array_push($paramData, Array($datum['value'], $datum['type'], $datum['component'], $datum['score']));
		}
		$st = $db->prepare(
			"INSERT INTO ". SQ_TABLE_PREFIX ."asset_indexing(value, type, assetid, component, score)
			VALUES (?, ?, $assetid, ?, ?)"
		);

		$db->executeMultiple($st, $paramData);
	} // end addKeyData()

	/**
	  * Adds a single piece of data for an asset.
	  * 
	  * @access private
	  * @param int assetid Asset ID.
	  * @param string $value The value of the keydata.
	  * @param string $type	Type of the keydata - (e.g. 'text', 'date', 'number')
	  * @param string $component The component of the asset to which this keydata pertains (defaults to '')
	  * @returns void
	*/

	function addKeyDatum($assetid, $value, $type = 'text', $component = '') 
	{
		if (!$assetid || !$value)
			return;
		if (is_null($type))
			$type = 'text';
		if (is_null($component))
			$component = '';


		$sql = "INSERT INTO ".SQ_TABLE_PREFIX."asset_indexing (value, type, assetid, component)
				VALUES (
					'". addslashes($value)		."', 
					'". addslashes($type)		."',
					'". addslashes($assetid)	."',
					'". addslashes($component)	."'
				)";

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$results = $db->query($sql);
	} // end addKeyDatum()

} // end class

?>
