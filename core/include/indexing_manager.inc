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
* $Id: indexing_manager.inc,v 1.9.2.2 2004/03/02 18:29:12 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Indexing_Manager
*
* Handles requests for indexing keydata from searchable assets, and handles search requests.
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.9.2.2 $ - 1.0
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

	}//end constructor


	/**
	* Signify that the keydata should be flushed and freshly retrieved
	*
	* @param int		$assetid	ID of the asset
	*
	* @return void
	* @access public
	*/
	function changed($assetid)
	{
		if (!$assetid) return;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// get dependent parents
		$changed_assetids = $am->getDependantParents($assetid);
		$changed_assetids[] = $assetid;

		foreach ($changed_assetids as $assetid) {
			$this->flushKeyData($assetid);

			$asset = &$am->getAsset($assetid);
			$new_key_data = $asset->getKeyData();

			$this->addKeyData($assetid, $new_key_data);
		}

	}//end changed()


	/**
	* Flush the keydata for an asset
	*
	* @param int		$assetid	ID of the asset
	*
	* @return void
	* @access public
	*/
	function flushKeyData($assetid)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'asset_indexing
				WHERE assetid = '.$db->quote($assetid);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		}

	}//end flushKeyData()


	/**
	* Adds keydata for an asset
	*
	* @param int		$assetid	ID of the asset
	* @param array		&$keydata	an array of the form :
	*<pre>
	*	Array (
	*		0 => Array (
	*				'value'     => ... ,
	*				'type'      => ... ,
	*				'component' => ... (optional, defaults to '')
	*				), ...
	*		)
	* </pre>
	* @access private
	* @returns void
	*/
	function addKeyData($assetid, &$keydata)
	{
		if (!is_array($keydata)) return;
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$paramData = array();
		foreach($keydata as $i => $datum) {
			// convert to ISO 8601 date format Y-m-d H:i:s if not already
			$datum =& $keydata[$i];
			if ($datum['type'] == 'date') {
				if (!(ereg('[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}', $datum['type']))) {
					$datum['value'] = date('Y-m-d H:i:s', strtotime($datum['value']));
				}
			} else if ($datum['type'] == 'text') {
				$datum['value'] = strtolower($datum['value']);
			}
			array_push($paramData, Array($datum['value'], $datum['type'], $datum['component'], $datum['score']));
		}

		$st = $db->prepare(
			'INSERT INTO '.SQ_TABLE_PREFIX.'asset_indexing(value, type, assetid, component, score)
			 VALUES (?, ?, '.$db->quote($assetid).', ?, ?)');

		$db->executeMultiple($st, $paramData);

	}//end addKeyData()


	/**
	* Add a single piece of data for an asset
	*
	* @param int		$assetid	ID of the asset
	* @param string		$value		the value of the keydata
	* @param string		$type		type of the keydata - (e.g. 'text', 'date', 'number')
	* @param string		$component	the component of the asset to which this keydata pertains (defaults to '')
	*
	* @return void
	* @access public
	*/
	function addKeyDatum($assetid, $value, $type='text', $component='')
	{
		if (!$assetid || !$value) return;
		if (is_null($type)) $type = 'text';
		if (is_null($component)) $component = '';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'asset_indexing (value, type, assetid, component)
				VALUES (
					'.$db->quote($value)    .',
					'.$db->quote($type)     .',
					'.$db->quote($assetid)  .',
					'.$db->quote($component).'
				)';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		}

	}//end addKeyDatum()


	/**
	* Sets the default component weighting for a particular asset type
	*
	* This does NOT create any new rows in defaults
	*
	* @param string		$type_code	the type of asset
	* @param string		$component	the component being wieghted
	* @param int		$weighting	the new weighting of the component
	*
	* @return void
	* @access public
	*/
	function setComponentDefaultWeighting($type_code, $component, $weighting)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if (strlen($component) == 0)
			continue;

		$sql = 'UPDATE '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
				SET weighting = ' . $db->quote($weighting) .'
				WHERE type_code = '. $db->quote($type_code) . ' AND component = '. $db->quote($component) ;

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		}

	}//end setComponentDefaultWeighting()


	/**
	* Sets the component weighting for a particular asset
	*
	* @param int				$assetid		the asset ID
	* @param int				$component		the component
	* @param mixed (null|int)	$weighting		the weighting - if null, reverts back to the default for this asset's type
	*
	* @return void
	* @access public
	*/
	function setComponentWeighting($assetid, $component, $weighting=null)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'asset_indexing_weightings
				WHERE assetid = '.$db->quote($assetid).' AND component = '.$db->quote($component);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
		}

		if (!is_null($weighting)) {
			$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'asset_indexing_weightings(assetid, component, weighting)
					VALUES ('.
						$db->quote($assetid)   .', '.
						$db->quote($component) .', '.
						$db->quote($weighting) .
					')';
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			}
		}

	}//end setComponentWeighting()


}// end class

?>
