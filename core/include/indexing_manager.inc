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
* $Id: indexing_manager.inc,v 1.8 2003/11/18 15:37:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Indexing_Manager
*
* Handles requests for indexing keydata from searchable assets, and handles search requests.

*
* @author  Dominic Wong<dwong@squiz.net>
* @version $Revision: 1.8 $ - 1.0
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

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		// get dependent parents
		$changed_assetids = $am->getDependantParents($assetid);
		$changed_assetids[] = $assetid;

		foreach ($changed_assetids as $assetid) {
			$this->flushKeyData($assetid, $component);

			$asset =& $am->getAsset($assetid);
			$newKeyData =& $asset->getKeyData($component);

			$this->addKeyData($assetid, $newKeyData);
		}
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
	function addKeyData($assetid, &$keydata) 
	{	
		if (!is_array($keydata))
			return;
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$paramData = array();
		foreach($keydata as $i => $datum) {
			# convert to ISO 8601 date format Y-m-d H:i:s if not already
			$datum =& $keydata[$i];
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

	/** 
	* Sets the default component weighting for a particular asset type. This does NOT create any new rows in defaults. 
	* Make sure component exists.
	*
	* 
	*/
	function setComponentDefaultWeighting ($type_code, $component, $weighting)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if (strlen($component) == 0)
			continue;

		$sql = 'UPDATE '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
				SET weighting = ' . $db->quote($weighting) .'
				WHERE type_code = '. $db->quote($type_code) . ' AND component = '. $db->quote($component) ;
	
		$db->query($sql);

	} // end setComponentDefaultWeighting()

	/** 
	* Sets the component weighting for a particular asset. 
	*
	* @param int $assetid				The asset id
	* @param int $component				The component
	* @param [null|int] $weighting		The weighting. If null, then this reverts back to the default for this asset's type.
	*/
	function setComponentWeighting ($assetid, $component, $weighting = null)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM '. SQ_TABLE_PREFIX .'asset_indexing_weightings 
				WHERE assetid = '. $db->quote($assetid) . ' AND component = '. $db->quote($component) ;
		$db->query($sql);
		if (!is_null($weighting)) {
			$sql = 'INSERT INTO '. SQ_TABLE_PREFIX .'asset_indexing_weightings(assetid, component, weighting) 
					VALUES ('. 
						$db->quote($assetid)				. ', ' .
						$db->quote($component)				. ', ' .
						$db->quote($weighting)	.
					')';
			$db->query($sql);
		}

	} // end setComponentWeighting()

} // end class

?>
