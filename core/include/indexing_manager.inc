<?php

/**
* Indexing_Manager
*
* Purpose
*    Handles requests for indexing keydata from searchable assets, and handles search requests.

*
* @author  Dominic Wong<dwong@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Indexing_Manager extends MySource_Object 
{
	function Indexing_Manager() 
	{
		parent::MySource_Object();
	}

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
	}

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
	}

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
			} else if ($datum['type'] == 'word') {
				$datum['value'] = strtolower($datum['value']);
			}

			array_push($paramData, Array($datum['value'], $datum['type'], $datum['component'], $datum['score']));
		}
		$st = $db->prepare(
			"INSERT INTO ". SQ_TABLE_PREFIX ."asset_indexing(value, type, assetid, component, score)
			VALUES (?, ?, $assetid, ?, ?)"
		);

		$db->executeMultiple($st, $paramData);
	}

	/**
	  * Adds a single piece of data for an asset.
	  * 
	  * @access private
	  * @param int assetid Asset ID.
	  * @param string $value The value of the keydata.
	  * @param string $type	Type of the keydata - (e.g. 'word', 'date', 'number')
	  * @param string $component The component of the asset to which this keydata pertains (defaults to '')
	  * @returns void
	*/

	function addKeyDatum($assetid, $value, $type = 'word', $component = '') 
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
	}

	function paintBackend(&$o)
	{
		// Lets get the asset types
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$typeHierarchy = $am->getAssetTypeHierarchy();

		// Now that we have the asset types, let's display the tree

		$o->openSection("Asset Weights");
		$fieldPrefix = $o->openField("Asset Types", '');

		echo "<ul>";
		foreach ($typeHierarchy as $typeCode => $assetType) {
			echo "<li><h2>". (ucfirst(str_replace("_", " ", $typeCode)))."</h2>";
			
			$typeInfo = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($typeCode);
			echo "<p><b>TypeInfo</b><ul>";
			foreach ($typeInfo as $key => $value) {
				echo "<li><b>" . ucfirst(str_replace("_", " ", $key)) . "</b> : " . $value."</li>";
			}
			echo "</ul></p>";

			$attributeids = $am->getAssetTypeAttributes($typeCode);
			echo "<p><b>Attributes</b><ul>";
			foreach ($attributeids as $attributeid) {
				$attribute = Asset_Attribute::loadAttribute($attributeid);
				$o->addHiddenField($fieldPrefix. "_weights[". $typeCode. "][". $attributeid. "]", $attribute->weighting);
				echo "<li>".$attribute->name." Weighting: ";
				echo text_box($fieldPrefix. "_new_weights[". $typeCode. "][". $attributeid. "]", $attribute->weighting, 1). "</li>";
			}
			echo "</ul></p>";
			echo "</li>";
		}
		echo "</ul>";

		$o->closeSection();

		$o->commitButton();
	}

	function processBackend(&$o)
	{
	}
}

?>
