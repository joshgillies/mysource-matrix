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
* $Id: search_manager.inc,v 1.23.2.3 2004/10/26 06:44:57 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/
require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types.
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.23.2.3 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Search_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Manager($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a search manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Event fired when an asset messages that it requires its stuff to be reindexed
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onRequestReindex(&$broadcaster, $vars=Array())
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return false;

		// if search manager is reindexed then there is infinite recursion!!!
		if ($broadcaster->id == $this->id) return false;

		$component = isset($vars['component']) ? $vars['component'] : null;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// get dependent parents - but put broadcaster first
		$changed_assetids = Array($broadcaster->id);
		$inform_parents = array_get_index($vars, 'inform_parents', true);
		if ($inform_parents) $changed_assetids += $am->getDependantParents($broadcaster->id);

		// for each of parents+current asset, refresh key data for specified component
		foreach ($changed_assetids as $assetid) {
			$asset = &$am->getAsset($assetid);
			$this->flushKeyData($assetid, $component);
			$new_key_data = $asset->getKeyData();
			$this->addKeyData($assetid, $new_key_data);
			$am->forgetAsset($assetid);
		}

		return true;

	}//end onRequireReindex()


	/**
	* Event fired when an asset gets deleted. We want to flush ank key data in the index table
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array		$vars		the vars that get submitted by the broadcaster
	*					Component is set as $vars['component']
	*
	* @return void
	* @access public
	*/
	function onAssetDeleted(&$broadcaster, $vars=Array())
	{
		if (is_null($broadcaster)) return;
		$this->flushKeyData($broadcaster->id);

	}//end onAssetDeleted()


	/**
	* Flush the keydata for an asset
	*
	* @param int		$assetid	ID of the asset
	* @param string		$component	the component to which the keydata pertains (e.g. metadata or attribute)
	*
	* @return void
	* @access public
	*/
	function flushKeyData($assetid, $component=null)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return;

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'asset_indexing
				WHERE assetid = '.$db->quote($assetid);
		if (!is_null($component)) $sql .= ' AND component = '.$db->quote($component);

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
	* @return boolean
	* @access public
	*/
	function addKeyData($assetid, &$keydata)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return false;
		if (!is_array($keydata)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$param_data = Array();

		$keys = array_keys($keydata);
		foreach ($keys as $key) {
			$datum =& $keydata[$key];
			if ($datum['type'] == 'date') {
				// convert to ISO 8601 date format Y-m-d H:i:s if not already
				if (!(ereg('[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}', $datum['type']))) {
					$datum['value'] = date('Y-m-d H:i:s', strtotime($datum['value']));
				}
			} else if ($datum['type'] == 'text') {
				$datum['value'] = strtolower($datum['value']);
			}

			$word = $datum['value'];

			// filter noise words
			if ($this->isWordIndexable($word)) continue;

			array_push($param_data, Array($datum['value'], $datum['type'], $datum['component'], $datum['score']));
		}

		if (empty($param_data)) return false;

		$st = $db->prepare(
			'INSERT INTO '.SQ_TABLE_PREFIX.'asset_indexing(value, type, assetid, component, score)
			 VALUES (?, ?, '.$db->quote($assetid).', ?, ?)');

		$result = $db->executeMultiple($st, $param_data);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}//end addKeyData()


	/**
	* Check if the word is indexable
	*
	* Check is performed according to the current indexing parameters
	*
	* @param string		$word	word to index
	*
	* @return boolean
	* @access public
	*/
	function isWordIndexable($word)
	{
		$min_length = $this->attr('min_word_length');
		return ((strlen($word) < $min_length) || ($this->isNoiseWord($word))) && (!$this->isWhiteWord($word));

	}//end isWordIndexable()


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


	/**
	* Retrieves weightings for this asset and its components - loads on demand.
	*
	* @return Array
	* @access public
	*/
	function &getAssetWeightings($assetid)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// load the weightings
		$sql = 'SELECT wd.component, wd.description, (w.weighting IS NULL) as def, COALESCE(w.weighting, wd.weighting) as weighting
				FROM
					'. SQ_TABLE_PREFIX .'asset_indexing_weightings_default wd
						LEFT OUTER JOIN '. SQ_TABLE_PREFIX. 'asset a
							ON wd.type_code = a.type_code
						LEFT OUTER JOIN '.SQ_TABLE_PREFIX .'asset_indexing_weightings w
							ON w.component = wd.component AND
							a.assetid = w.assetid
				WHERE
					a.assetid = ' . $db->quote($assetid) ;

		$results = $db->getAll($sql);
		if (DB::isError($results)) {
			trigger_error($results->getMessage().'<br/>'.$results->getUserInfo(), E_USER_ERROR);
			return false;
		}

		$weightings = Array();
		foreach ($results as $result) {
			$weightings[$result['component']] =
				Array(
					'default'     => ($result['def'] == 't') ? 1 : 0,
					'weighting'   => $result['weighting'],
					'description' => $result['description'],
				);
		}

		return $weightings;

	}// end getAssetWeightings()


//--        METADATA INDEXING        --//


	/**
	* Event fired when metadata manager wants some metadata reindexed
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event (should be meta. mgr)
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										Asset ID is set as $vars['assetid']
	*										Schema Asset IDs is set as $vars['schemaids']
	*
	* @access public
	* @return void
	*/
	function onIndexMetadata(&$broadcaster, $vars=Array())
	{
		$assetid = $vars['assetid'];
		$schemaids =& $vars['schemaids'];
		// TODO: This needs to be split out
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		foreach ($schemaids as $schemaid) {
			$sectionids = $am->getChildren($schemaid, 'metadata_section', false);
			foreach ($sectionids as $sectionid => $section_type) {
				$fieldids = $am->getChildren($sectionid, 'metadata_field', false);
				foreach ($fieldids as $fieldid => $field_type) {
					$this->flushKeyData($assetid, 'metadata:'.$fieldid);
				}
			}
		}
		$this->addKeyData($assetid, $this->getMetadataKeyData($assetid, $schemaids));

	}//end indexMetadata()


	/**
	* Builds key data array for indexing, with keyword replacements.
	*
	* @param Object		&$asset			The asset
	* @param Array		&$metadata		The values for the metadata
	* @param Array		&$schemaids		The schema IDs
	*
	* @return Array
	* @access private
	*/
	function getMetadataKeyData($assetid, $schemaids)
	{
		// get the default values
		$keydata = Array();
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$asset =& $am->getAsset($assetid);
		foreach ($schemaids as $schemaid) {
			$sectionids = $am->getChildren($schemaid, 'metadata_section', false);
			$metadata = $mm->getSchemaMetadata($assetid, $schemaid);

			foreach ($sectionids as $sectionid => $section_type) {
				$section = &$am->getAsset($sectionid, $section_type);

				$fieldids = $am->getChildren($sectionid, 'metadata_field', false);
				foreach ($fieldids as $fieldid => $field_type) {

					$field =& $am->getAsset($fieldid, $field_type);

					if (isset($metadata['sections'][$section->name]['fields'][$field->name]['value'])) {
						$value = $metadata['sections'][$section->name]['fields'][$field->name]['value'];
					} else {
						$value = $field->attr('default');
					}

					$force_processing = 1; // will tell the function to process values in any case

					// check field_type to define the indexing way
					if ($field_type == 'metadata_field_date') {

						$mydate = date('Y-m-d H:i:s', $value);
						$asset->buildKeyData($keydata, $mydate, 'datetime', 'metadata:'. $field->id, $field->attr('weighting') * 1, $force_processing);

					} else {

						$keywords = retrieve_keywords_replacements($value, '.');

						$replacements = $mm->generateKeywordReplacements($asset, $keywords, false);
						foreach ($keywords as $keyword => $replacement) {
							$replacements[$keyword] = strtolower($replacement);
						}
						$value = replace_keywords($value, $replacements);

						$word_counts = get_word_counts(strtolower($value));

						foreach ($word_counts as $word => $count) {
							$asset->buildKeyData($keydata, $word, 'text', 'metadata:'. $field->id, $field->attr('weighting') * $count, $force_processing);
						}

					}
				}
			}
		}
		return $keydata;

	}//end getMetadataKeyData()


//--        KEYWORD EXTRACTION        --//


	/**
	* Extracts keywords from the indexing table and ranks them in order of descending importance
	*
	* @param object Asset	&$asset	The asset whose keywords are to be extracted
	* @param boolean	$include_metadata	If false, metadata components are excluded
	* @param boolean	$include_scores	Determines whether scores are returned (true) or just
	*									the keywords (false). Affects return value.
	*
	* @return Array	depends on $include_scores:
	*				false: Array(string) containing keywords ordered in decreasing total score
	*				true: Array(string => int) score array indexed by keyword, ordered same way
	* @access public
	*/
	function extractKeywords(&$asset, $include_metadata=false, $include_scores=false)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT LOWER(value) as keyword, SUM(score) as total_score
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_indexing
				WHERE assetid = '.$db->quote($asset->id).
				  ($include_metadata ? '' : ' AND component NOT LIKE '.$db->quote('metadata:%')).'
				GROUP BY keyword ORDER BY total_score DESC';

		$result = $db->getAssoc($sql);
		assert_valid_db_result($result);

		// force lowercase keys (= keywords) - CASE_LOWER is default for this function
		$result = array_change_key_case($result);

		if ($include_scores) {
			return $result;				// return result with scores
		} else {
			return array_keys($result);	// just return the keywords
		}

	}//end extractKeywords()


//--        SEARCH PROCESSING        --//


	/**
	* Processes a search request
	*
	* Formulates search query and returns the results
	*
	* @param array	$search_info	an array of search settings in the form
	* <PRE>
	* Array('root_assetid'    => asset ID to search under,
	*       'logic'           => 'OR'|'AND',
	*       'search_statuses' => Array( ..statues to search.. ),
	*       'search_types'    => Array( ..asset types to search.. ),
	*       'sort_by'         => 'name'|'short_name'|'created'|'updated'|'published'|'type_code',
	*       'include_all'     => Array(ATTR_NAME => Array('words'  => Array( ..search terms.. )),
	*       'exclude'         => Array(ATTR_NAME => Array('words'  => Array( ..search terms.. )),
	*       'asset_attrib'    => Array(ATTR_NAME => Array(
	*                                                  'words'  => Array( ..search terms.. ),
	*                                                  'params' => Array('searchtype' => '',
	*                                                                    'type_attr' => 'NAME_TYPE_ID'
	*                                                                   )
	*                                                    ),
	*       'metadata'        => Array(ATTR_NAME => Array(
	*                                                  'words'  => Array( ..search terms.. ),
	*                                                  'params' => Array('assetid' => FIELD_ID)
	*                                                    )
	* );
	*</PRE>
	*
	* @return array
	* @access public
	*/
	function processSearch($search_info)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$total_asset_scores = null;


		  /////////////////////////////
		 //  CONSTRUCT CACHING KEY  //
		/////////////////////////////
		$search_cache_key = $search_info['root_assetid'];
		if (isset($search_info['search_statuses'])) $search_cache_key .= implode(':', $search_info['search_statuses']);
		if (isset($search_info['search_types']))    $search_cache_key .= implode(':', $search_info['search_types']);
		if (isset($search_info['sort_by']))         $search_cache_key .= $search_info['sort_by'];

		// we need the cache manager to cache the results of queries
		$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');


		  ///////////////////////////////////
		 //  CONSTRUCT BASE SEARCH QUERY  //
		///////////////////////////////////
		$start_sql_array = $this->constructBaseSearchQuery($search_info);
		$start_sql = implode(' ', $start_sql_array);
		$group_by =  'GROUP BY ai.assetid';
		if (isset($search_info['sort_by'])) $group_by .= ', sort_by';


		  ///////////////////////////////
		 //  SEARCH ASSET ATTRIBUTES  //
		///////////////////////////////
		if (isset($search_info['asset_attrib'])) {

			foreach($search_info['asset_attrib'] as $attr_name => $attr_info) {

				if ($attr_info['params']['asset_type'] == '') continue;
				$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($attr_info['params']['attrid']);
				if (is_null($attribute)) continue;

				$field_asset_scores = null;

				switch ($attribute->type()) {
					case 'datetime' :
						$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
						$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

						$where = '';
						if ($from != '---------- --:--:--') $where .= ' AND ai.value >= '.$db->quote($from);
						if ($to != '---------- --:--:--') $where .= ' AND ai.value <= '.$db->quote($to);
						if (empty($where)) continue;
						if (empty($where)) continue;

						$result = $cm->loadFromCache($this, $search_cache_key.'asset_attrib'.$from.'_'.$to);
						if (empty($result)) {
							$sql = $start_sql.'
									AND ai.type = '.$db->quote('datetime').'
									AND ai.component = '.$db->quote('attr:'. $attr_info['params']['asset_type'].':'.$attribute->name).'
									'.$where.' '.$group_by;

							$result = $db->getAssoc($sql);
							assert_valid_db_result($result);

							$cm->saveToCache($this, $search_cache_key.'asset_attrib'.$from.'_'.$to, serialize($result));
						} else {
							$result = unserialize($result);
						}

						$field_asset_scores = $result;
					break;

					default :
						$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
						if ($field_search_logic != 'AND' && $field_search_logic != 'OR') $field_search_logic = 'AND';

						foreach ($attr_info['words'] as $word) {
							$result = $cm->loadFromCache($this, $search_cache_key.'asset_attrib'.$word);
							if (empty($result)) {
								$sql = $start_sql.'
									AND ai.component = '.$db->quote('attr:'. $attr_info['params']['asset_type'].':'.$attribute->name).'
									AND (ai.value LIKE '.$db->quote('%'.$word.'%').') '.$group_by;

								$result = $db->getAssoc($sql);
								assert_valid_db_result($result);

								$cm->saveToCache($this, $search_cache_key.'asset_attrib'.$word, serialize($result));
							} else {
								$result = unserialize($result);
							}

							if (is_null($field_asset_scores)) {
								$field_asset_scores = $result;
							} else {
								$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic);
							}
						}
					break;
				}//end switch attribute type

				if (is_null($total_asset_scores)) $total_asset_scores = $field_asset_scores;
				else $total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  //////////////////////////
		 //  SEARCH ASSET DATES  //
		//////////////////////////
		if (isset($search_info['asset_date'])) {

			$valid_dates = Array('created', 'updated', 'published');

			foreach($search_info['asset_date'] as $attr_name => $attr_info) {

				if (!isset($attr_info['params']['date_type'])) continue;
				if (!in_array($attr_info['params']['date_type'], $valid_dates)) continue;

				$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
				$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

				$where = '';
				if ($from != '---------- --:--:--') $where .= ' AND ai.value >= '.$db->quote($from);
				if ($to != '---------- --:--:--') $where .= ' AND ai.value <= '.$db->quote($to);
				if (empty($where)) continue;

				$result = $cm->loadFromCache($this, $search_cache_key.'asset_date'.$from.'_'.$to);
				if (empty($result)) {
					$sql = $start_sql.'
							AND ai.type = '.$db->quote('datetime').'
							AND ai.component = '.$db->quote('__'.$attr_info['params']['date_type'].'__').'
							'.$where.' '.$group_by;

					$result = $db->getAssoc($sql);
					assert_valid_db_result($result);

					$cm->saveToCache($this, $search_cache_key.'asset_date'.$from.'_'.$to, serialize($result));
				} else {
					$result = unserialize($result);
				}

				if (is_null($total_asset_scores)) $total_asset_scores = $result;
				else $total_asset_scores = $this->combineAssetScores($total_asset_scores, $result, $search_info['logic']);
			}
		}


		  /////////////////////////////////
		 //  SEARCH INCLUDE ALL FIELDS  //
		/////////////////////////////////
		if (isset($search_info['include_all'])) {

			foreach($search_info['include_all'] as $attr_name => $attr_info) {

				$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
				if ($field_search_logic != 'AND' && $field_search_logic != 'OR') $field_search_logic = 'AND';

				$field_asset_scores = null;
				foreach ($attr_info['words'] as $word) {
					$result = $cm->loadFromCache($this, $search_cache_key.'include_all'.$word);
					if (empty($result)) {
						$sql = $start_sql.'
							AND (ai.value LIKE '.$db->quote('%'.$word.'%').') '.$group_by;

						$result = $db->getAssoc($sql);
						assert_valid_db_result($result);

						$cm->saveToCache($this, $search_cache_key.'include_all'.$word, serialize($result));
					} else {
						$result = unserialize($result);
					}

					if (is_null($field_asset_scores)) {
						$field_asset_scores = $result;
					} else {
						$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic);
					}
				}

				if (is_null($total_asset_scores)) $total_asset_scores = $field_asset_scores;
				else $total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  ///////////////////////
		 //  SEARCH METADATA  //
		///////////////////////
		if (isset($search_info['metadata'])) {

			foreach($search_info['metadata'] as $attr_name => $attr_info) {

				if ($attr_info['params']['assetid'] <= 0) continue;
				$metadata_field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($attr_info['params']['assetid']);
				if (is_null($metadata_field)) continue;

				$field_asset_scores = null;

				switch (get_class($metadata_field)) {
					case 'metadata_field_date' :
						$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
						$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

						$where = '';
						if ($from != '---------- --:--:--') $where .= ' AND ai.value >= '.$db->quote($from);
						if ($to != '---------- --:--:--') $where .= ' AND ai.value <= '.$db->quote($to);
						if (empty($where)) continue;

						$result = $cm->loadFromCache($this, $search_cache_key.'metadata'.$from.'_'.$to);
						if (empty($result)) {
							$sql = $start_sql.'
									AND ai.type = '.$db->quote('datetime').'
									AND ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['assetid']).'
									'.$where.' '.$group_by;

							$result = $db->getAssoc($sql);
							assert_valid_db_result($result);

							$cm->saveToCache($this, $search_cache_key.'metadata'.$from.'_'.$to, serialize($result));
						} else {
							$result = unserialize($result);
						}

						if (is_null($field_asset_scores)) $field_asset_scores = $result;
						else $field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, 'AND');
					break;

					default :
						$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
						if ($field_search_logic != 'AND' && $field_search_logic != 'OR') $field_search_logic = 'AND';

						foreach ($attr_info['words'] as $word) {
							$result = $cm->loadFromCache($this, $search_cache_key.'metadata'.$word);
							if (empty($result)) {
								$sql = $start_sql.'
										AND ai.type = '.$db->quote('text').'
										AND ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['assetid']).'
										AND ai.value LIKE '.$db->quote('%'.$word.'%').' '.$group_by;

								$result = $db->getAssoc($sql);
								assert_valid_db_result($result);

								$cm->saveToCache($this, $search_cache_key.'metadata'.$word, serialize($result));
							} else {
								$result = unserialize($result);
							}

							if (is_null($field_asset_scores)) {
								$field_asset_scores = $result;
							} else {
								$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic);
							}
						}//end foreach word
					break;
				}

				if (is_null($total_asset_scores)) $total_asset_scores = $field_asset_scores;
				else $total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  /////////////////////////////
		 //  SEARCH EXCLUDE FIELDS  //
		/////////////////////////////
		if (isset($search_info['exclude'])) {

			foreach($search_info['exclude'] as $attr_name => $attr_info) {

				$sql = $start_sql.'
					AND (ai.value LIKE \'%'.implode("%' OR ai.value LIKE '%", $attr_info['words']).'%\') '.$group_by;

				$result = $db->getAssoc($sql);
				assert_valid_db_result($result);

				foreach (array_keys($result) as $exclude_assetid) {
					if (isset($total_asset_scores[$exclude_assetid])) unset($total_asset_scores[$exclude_assetid]);
				}
			}
		}

		if (is_null($total_asset_scores)) {
			$total_asset_scores = Array();
		} else {
			if (empty($search_info['sort_by'])) {
				// sorting by relevance, so ensure the sort_by score is the same as the actual search score
				for (reset($total_asset_scores); null !== ($assetid = key($total_asset_scores)); next($total_asset_scores)) {
					$total_asset_scores[$assetid]['sort_by'] = $total_asset_scores[$assetid]['search_score'];
				}
			}
			$sorted_scores = Array();
			foreach ($total_asset_scores as $assetid => $search_data) {
				$sorted_scores[strtolower($search_data['sort_by'])][$assetid] = $search_data['search_score'];
			}
			if (isset($search_info['reverse_sort']) && $search_info['reverse_sort']) {
				krsort($sorted_scores);
			} else {
				ksort($sorted_scores);
			}
			$total_asset_scores = Array();
			foreach ($sorted_scores as $sort_by => $results) {
				foreach ($results as $assetid => $search_score) $total_asset_scores[$assetid] = $search_score;
			}
		}
		return $total_asset_scores;

	}//end processSearch()


	/**
	* Processes a basic search request without search terms
	*
	* Accepts the same input array as processSearch but does use any search terms
	*
	* @param array	$search_info	an array of search settings
	*
	* @return array
	* @access public
	* @see processSearch()
	*/
	function processBasicSearch($search_info)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$total_asset_scores = null;
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->currentUserId() || (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()));


		$concat = ($db->phptype == 'mysql') ? 'CONCAT(alt1.treeid, '.$db->quote('%').')' : '(alt1.treeid || '.$db->quote('%').')';

		$search_sql = 'SELECT DISTINCT(a.assetid), 1 AS score
					   FROM
							'.SQ_TABLE_PREFIX.'asset_link_tree alt1,
							'.SQ_TABLE_PREFIX.'asset_link_tree alt2,
							'.SQ_TABLE_PREFIX.'asset_link al1,
							'.SQ_TABLE_PREFIX.'asset_link al2,
							'.SQ_TABLE_PREFIX.'asset a';
		if ($user_restrictions) $search_sql .= ', '.SQ_TABLE_PREFIX.'asset_permission ap ';

		$search_sql .= ' WHERE ';

		if ($user_restrictions) $search_sql .= 'a.assetid = ap.assetid AND ';

		$search_sql .= ' al1.majorid = '.$db->quote($search_info['root_assetid']).'
						 AND al1.linkid  = alt1.linkid
						 AND alt2.treeid LIKE '.$concat.'
						 AND alt2.linkid = al2.linkid
						 AND a.assetid  = al2.minorid';

		// add in status restrictions
		if (isset($search_info['search_statuses']) && !empty($search_info['search_statuses'])) {
			$statues = $search_info['search_statuses'];
			for($i = 0; $i < count($statues); $i++) $statues[$i] = $db->quote($statues[$i]);
			$search_sql .= ' AND a.status IN ('.implode(', ', $statues).')';
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$search_sql .= ' AND a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		if (!empty($search_info['search_types'])) {
			for(reset($search_info['search_types']); NULL !== ($i = key($search_info['search_types'])); next($search_info['search_types'])) {
				$search_info['search_types'][$i] = $db->quote($search_info['search_types'][$i]);
			}
			$type_code_cond = 'IN ('.implode(', ', $search_info['search_types']).')';

			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$search_sql .= ' AND a.type_code IN (~SQ0~)';
			$subs = Array( 'SELECT type_code
							FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
							WHERE inherited_type_code '.$type_code_cond );
			$search_sql = db_extras_subquery($db, $search_sql, $subs);
			if (DB::isError($search_sql)) {
				trigger_error($search_sql->getMessage().'<br/>'.$search_sql->getUserInfo(), E_USER_ERROR);
			}
		}

		$userid_cond = '';
		if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$userids = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false);
				array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
				for(reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote($userids[$i]);
				}
				$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
			}
		} else {
			// public users
			$userid_cond = ' AND ap.userid = 0';
		}

		if (!empty($userid_cond)) {
			$search_sql .= $userid_cond;
			$search_sql .= ' AND ap.permission >= '.$db->quote(SQ_PERMISSION_READ).'
							 AND ap.access = 1';
		}

		$search_results = Array();
		if (isset($search_info['limit'])) {
			// limiting the search results
			$result = $db->limitQuery($search_sql, 0, $search_info['limit']);
			assert_valid_db_result($result);

			while ($result->fetchInto($row, DB_FETCHMODE_ASSOC)) {
				$search_results[$row['assetid']] = 1;
			}
		} else {
			$search_results = $db->getAssoc($search_sql);
			assert_valid_db_result($search_results);
		}

		return $search_results;

	}//end processBasicSearch()


	/**
	* Generates a base query for searching
	*
	* Accepts the same search info array as processSearch()
	*
	* @param array	$search_info	an array of search settings
	*
	* @return array
	* @access public
	* @see processSearch()
	*/
	function constructBaseSearchQuery($search_info)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->currentUserId() || (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()));

		// sorting by relevance by default
		$sort_string = 'SUM(ai.score)';

		if (isset($search_info['sort_by'])) {
			$valid_sort_fields = Array('name', 'short_name', 'created', 'updated', 'published', 'type_code');
			if (in_array($search_info['sort_by'], $valid_sort_fields)) {
				// sorting by a field in the asset table
				$sort_string = 'a.'.$search_info['sort_by'];
			}
		}

		$concat = ($db->phptype == 'mysql') ? 'CONCAT(alt1.treeid, '.$db->quote('%').')' : '(alt1.treeid || '.$db->quote('%').')';

		$sql = Array();

		$sql['select'] = 'SELECT DISTINCT(ai.assetid), SUM(ai.score) as search_score, '.$sort_string.' AS sort_by';
		$sql['from']   = 'FROM
							'.SQ_TABLE_PREFIX.'asset_indexing ai,
							'.SQ_TABLE_PREFIX.'asset_link_tree alt1,
							'.SQ_TABLE_PREFIX.'asset_link_tree alt2,
							'.SQ_TABLE_PREFIX.'asset_link al1,
							'.SQ_TABLE_PREFIX.'asset_link al2,
							'.SQ_TABLE_PREFIX.'asset a';
		if ($user_restrictions) $sql['from'] .= ', '.SQ_TABLE_PREFIX.'asset_permission ap ';

		$sql['where'] = 'WHERE ';

		if ($user_restrictions) $sql['where'] .= 'a.assetid = ap.assetid AND ';

		$sql['where'] .= '  a.assetid = ai.assetid
							AND al1.majorid = '.$db->quote($search_info['root_assetid']).'
							AND al1.linkid  = alt1.linkid
							AND alt2.treeid LIKE '.$concat.'
							AND alt2.linkid = al2.linkid
							AND ai.assetid  = al2.minorid';

		// add in status restrictions
		if (isset($search_info['search_statuses']) && !empty($search_info['search_statuses'])) {
			$statues = $search_info['search_statuses'];
			for($i = 0; $i < count($statues); $i++) $statues[$i] = $db->quote($statues[$i]);
			$sql['where'] .= ' AND a.status IN ('.implode(', ', $statues).')';
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$sql['where'] .= ' AND a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		if (!empty($search_info['search_types'])) {
			for(reset($search_info['search_types']); NULL !== ($i = key($search_info['search_types'])); next($search_info['search_types'])) {
				$search_info['search_types'][$i] = $db->quote($search_info['search_types'][$i]);
			}
			$type_code_cond = 'IN ('.implode(', ', $search_info['search_types']).')';

			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$sql['where'] .= ' AND a.type_code IN (~SQ0~)';
			$subs = Array( 'SELECT type_code
							FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
							WHERE inherited_type_code '.$type_code_cond );
			$sql['where'] = db_extras_subquery($db, $sql['where'], $subs);
			assert_valid_db_result($sql['where']);
		}

		$userid_cond = '';
		if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$userids = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false);
				array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
				for(reset($userids); NULL !== ($i = key($userids)); next($userids)) {
					$userids[$i] = $db->quote($userids[$i]);
				}
				$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
			}
		} else {
			// public users
			$userid_cond = ' AND ap.userid = 0';
		}

		if (!empty($userid_cond)) {
			$sql['where'] .= $userid_cond;
			$sql['where'] .= ' AND ap.permission >= '.$db->quote(SQ_PERMISSION_READ).'
							AND ap.access = 1';
		}

		return $sql;

	}//end constructBaseSearchQuery()


	/**
	* Combines asset scores according to the passed logic
	*
	* Returns a mapping from assetids to combined scores
	*
	* @param array	$asset_scores1		a mapping from assetids to scores
	* @param array	$asset_scores2		a mapping from assetids to scores
	* @param string	$logic				either 'AND' or 'OR'
	*
	* @return array
	* @access private
	*/
	function combineAssetScores($asset_scores1, $asset_scores2, $logic='AND')
	{
		if (!is_array($asset_scores1) && !is_array($asset_scores2)) return Array();
		else if (!is_array($asset_scores2)) return $asset_scores1;
		else if (!is_array($asset_scores1)) return $asset_scores2;

		// find the intersection/union (depending on logic) of the assetids in field_asset_scores and results
		$assets1 = array_keys($asset_scores1);
		$assets2 = array_keys($asset_scores2);

		$out_asset_scores = $asset_scores1;

		if ($logic == 'AND') {

			$common_assets = array_intersect($assets1, $assets2);

			// get the assets that are to be removed in field_asset_scores
			$removed_assets = array_diff($assets1, $assets2);
			foreach ($removed_assets as $assetid) unset ($out_asset_scores[$assetid]);

			foreach ($common_assets as $assetid) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid]['search_score'] += $asset_scores2[$assetid]['search_score'];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}
			}

		} else if ($logic == 'OR') {

			foreach ($asset_scores2 as $assetid => $data) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid]['search_score'] += $asset_scores2[$assetid]['search_score'];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}
			}

		}

		return $out_asset_scores;

	}//end combineAssetScores()


	/**
	* Return an array of noise words
	*
	* @return array
	* @access public
	*/
	function getNoiseWords()
	{
		return $this->attr('noise_word_list');

	}//end getNoiseWords()


	/**
	* Return an array of white words
	*
	* @return array
	* @access public
	*/
	function getWhiteWords()
	{
		return $this->attr('white_word_list');

	}//end getWhiteWords()


	/**
	* Return true if word is in noise words
	*
	* @return boolean
	* @access public
	*/
	function isNoiseWord($word)
	{
		$words = $this->attr('noise_word_list');
		return isset($words[$word]);

	}//end isNoiseWord()


	/**
	* Return true if word is in white words
	*
	* @return boolean
	* @access public
	*/
	function isWhiteWord($word)
	{
		$words = $this->attr('white_word_list');
		return isset($words[$word]);

	}//end isWhiteWord()


	/**
	* Return the minimum indexable word lenght
	*
	* @return int
	* @access public
	*/
	function getMinWordLength()
	{
		return $this->attr('min_word_length');

	}//end getMinWordLength()


}//end class

?>
