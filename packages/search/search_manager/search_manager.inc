<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: search_manager.inc,v 1.62 2005/07/21 01:02:31 dheppell Exp $
*
*/
require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types.
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.62 $
* @package MySource_Matrix_Packages
* @subpackage search
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
		if (!system_asset_fns_create_pre_check($this)) {
			return false;
		}
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


//--        ASSET INDEXING        --//


	/**
	* Event fired when an asset messages that it requires its internals
	* to be reindexed
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onAssetUpdate(&$broadcaster, $vars=Array())
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return false;
		$this->reindexAsset($broadcaster, Array('asset'));

		return true;

	}//end onAssetUpdate()


	/**
	* Reindex this asset and, if specified, the metadata associated with this asset
	*
	* @param &object Asset		$asset		the asset to reindex
	* @param Array				$components	the components to reindex - 'asset' to
	*										reindex the core components of the asset, and
	*										'metadata' to reindex metadata or 'all' for both
	*
	* @return void
	* @access public
	*/
	function reindexAsset(&$asset, $components=Array('all'))
	{
		// if search manager is reindexed then there is infinite recursion!!!
		if ($asset->id == $this->id) return false;

		// check to see that there is an asset to index this data under
		if (!$indexing_assetid = $this->getIndexingAssetid($asset)) {
			return false;
		}

		if (in_array('all', $components) || in_array('asset', $components)) {
			$this->flushIndexableContent($indexing_assetid);
			$indexing_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($indexing_assetid);
			$new_key_data = $this->getIndexableContent($indexing_asset);
			$this->addIndexableContent($indexing_assetid, $new_key_data);
		}
		if (in_array('all', $components) || in_array('metadata', $components)) {
			$this->reindexMetadata($indexing_assetid);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($assetid);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($indexing_asset);

	}//end reindexAsset()


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
		return $this->flushIndexableContent($broadcaster->id);

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
	function flushIndexableContent($assetid, $component=null)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return;

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM
					sq_sch_idx
				WHERE
					assetid = '.$db->quoteSmart($assetid);
		if (!is_null($component)) {
			$sql .= ' AND component = '.$db->quoteSmart($component);
		}

		$result = $db->query($sql);
		assert_valid_db_result($result);

	}//end flushIndexableContent()


	/**
	* Returns the assetid in which content should be indexed under for the specified asset
	*
	* @param &object Asset	$asset	the asset to find the indexing asset for
	*
	* @return mixed the assetid or false if there is no indexing asset
	* @access public
	*/
	function getIndexingAssetid(&$asset)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, '', false, 'minor');
		if (empty($links)) return false;

		$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id);
		$parents = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($parents, Array(), false, 'type_code');
		if (empty($parents)) return $asset->id;

		// the first dependant parent will be the first element on the array
		$parent = array_shift(array_keys($parents));

		return $parent;

	}//end getIndexingAssetid()


	/**
	* Returns the indexable content for the specified asset
	*
	* @param &object	Asset $asset	the asset to obtain the dineavle content for
	*
	* @return Array
	* @access public
	*/
	function getIndexableContent(&$asset)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$index_content = $this->getIndexableMetadataContent($asset);
		$weightings    = $this->getWeightings($asset->id);

		$type_code = $asset->type();

		// index the name and the short name
		foreach (Array('name', 'short_name') as $name_type) {
			$name_index = '__'.$name_type.'__';
			// if there are no weightings, we dont want to index this name
			if (!isset($weightings[$name_index])) continue;

			$name = ($name_type == 'name') ? strtolower($asset->name) : strtolower($asset->short_name);
			foreach (get_word_counts($name) as $word => $count) {
				$index_content[] = Array(
									'value'		=> $word,
									'type_code'	=> $type_code,
									'type'		=> 'text',
									'component'	=> $name_index,
									'score'		=> $count * $weightings[$name_index],
								   );
			}
		}

		$dates = Array(
					'__created__'	=> $asset->created,
					'__updated__'	=> $asset->updated,
				 );
		if (!is_null($this->published)) {
			$dates['__published__'] = $asset->published;
		}
		if (!is_null($this->status_changed)) {
			$dates['__status_changed__'] = $asset->status_changed;
		}

		foreach ($dates as $type => $date) {
			// if there are no weightings, we don't want to index this date
			if (!isset($weightings[$type])) continue;

			$index_content[] = Array(
								'value'		=> ts_iso8601($date),
								'type_code'	=> $type_code,
								'type'		=> 'datetime',
								'component'	=> $type,
								'score'		=> $weightings[$type],
							   );
		}

		// only index this if there is a weighting set
		if (isset($weightings['contents'])) {
			// ask the asset for its content, and store it under a contents component
			// we don't want to index any tags so strip them
			$contents = strtolower(strip_tags(html_entity_decode($asset->getContent())));

			if (trim($contents) != '') {
				foreach (get_word_counts($contents) as $word => $count) {
					$index_content[] = Array(
										'value'		=> $word,
										'type_code'	=> $type_code,
										'type'		=> 'text',
										'component'	=> 'attr:contents',
										'score'		=> $count * $weightings['contents'],
									   );
				}
			}
		}

		foreach ($asset->vars as $var_name => $var_data) {

			// if there is no weightings for this attribute, then its not indexable
			if (!isset($weightings[$var_name])) continue;

			// don't index attribute that are names or short names as we index these
			// as part of the asset components
			if ($var_name == 'name' || $var_name == 'short_name') {
				continue;
			}

			// don't index passwords
			if ($var_data['type'] == 'password') continue;

			$attribute = &$asset->getAttribute($var_name);
			$value = strtolower($attribute->getContent());
			if (empty($value)) continue;

			$comp_name = 'attr:'.$var_name;

			foreach (get_word_counts($value) as $word => $count) {
				$index_content[] = Array(
									'value'		=> $word,
									'type_code'	=> $type_code,
									'type'		=> $var_data['type'],
									'component'	=> $comp_name,
									'score'		=> $count * $weightings[$var_name],
								   );
			}
		}//end foreach asset vars

		return $index_content;

	}//end getIndexableContent()


	/**
	* Adds the sepecifed index content for the specified assetid to the indexing system
	*
	* @param string	$assetid		the assetid to index the content under
	* @param Array	$index_content	the content to index
	*
	* @return boolean
	* @access public
	*/
	function addIndexableContent($assetid, &$index_content)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return false;
		if (!is_array($index_content)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$param_data = Array();

		$keys = array_keys($index_content);
		foreach ($keys as $key) {
			$data = &$index_content[$key];
			$value = $data['value'];

			if (trim($value) == '') continue;

			switch ($data['type']) {
				case 'datetime':
					require_once SQ_FUDGE_PATH.'/general/datetime.inc';
					if (!is_iso8601($value)) {
						$value = ts_iso8601(strtotime($value));
					}
				break;

				case 'text':
					// filter noise words
					if (!$this->isWordIndexable($value)) continue 2; //continue the foreach loop

					require_once SQ_FUDGE_PATH.'/general/text.inc';
					foreach (get_word_counts($value) as $word => $count) {
						$param_data[] = Array(
											$word,
											$data['type_code'],
											$data['type'],
											$data['component'],
											$count * $data['score'],
										);
					}
					continue 2;

				// NOTE: break is omitted on purpose
				case 'thesaurus':
				case 'select':
					$value = strtolower($value);
				break;

				default:
					// nothing yet
			}

			$param_data[] = Array(
								$value,
								$data['type_code'],
								$data['type'],
								$data['component'],
								$data['score'],
							);
		}

		if (empty($param_data)) return false;

		$st = $db->prepare(
				'INSERT INTO
					sq_sch_idx
					(
						value,
						type_code,
						type,
						assetid,
						component,
						score
					)
					VALUES
					(
						?,
						?,
						?,
						'.$db->quote($assetid).',
						?,
						?
					)'
				);

		assert_valid_db_result($st);
		$result = $db->executeMultiple($st, $param_data);
		assert_valid_db_result($result);

		$db->freePrepared($st);

		return true;

	}//end addIndexableContent()


	/**
	* Returns a list of indexable compents for all assets
	*
	* @return Array ($component => $display_name)
	* @access public
	*/
	function getIndexableAssetComponents()
	{
		$components = Array(
						'__name__'				=> 'Asset Name',
						'__short_name__'		=> 'Asset Short Name',
						'__created__'			=> 'Created Date',
						'__updated__'			=> 'Updated Date',
						'__published__'			=> 'Published Date',
						'__status_changed__'	=> 'Status Changed Date',
						'contents'				=> 'Asset Contents',
					  );

		return $components;

	}//end getIndexableAssetComponents()


	/**
	* Returns a list of indexable components for a specified asset type
	*
	* @param string	$type_code the type code
	*
	* @return Array
	* @access public
	*/
	function getIndexableAttributes($type_code)
	{
		$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($type_code, Array('name', 'type', 'is_admin'));

		$indexable_attrs = Array();
		foreach ($attrs as $attr => $info) {
			if (!$attrs[$attr]['is_admin'] || $attrs[$attr]['is_admin'] == 'f') {
				$indexable_attrs[$attr] = $info;
			}
		}
		return $indexable_attrs;

	}//end getIndexableAttributes()


	/**
	* Returns TRUE if the specified attribute is indexable
	*
	* @param int	$attrid	the attribute id
	*
	* @return boolean
	* @access public
	*/
	function isAttributeIndexable($attrid)
	{
		$info = $GLOBALS['SQ_SYSTEM']->am->getAttributeInfo($attrid);
		return !$info[$attrid]['is_admin'];

	}//end isAttributeIndexable()


	/**
	* Returns TRUE if the specified asset is indexable
	*
	* @param string	$assetid	the assetid to check
	*
	* @return boolean
	* @access public
	*/
	function isAssetIndexable($assetid)
	{
		$weights = $this->getAssetWeightings($assetid);
		return (isset($weights['indexed']) && $weights['indexed']);

	}//end isAssetIndexable()


	/**
	* Returns TRUE if the specified asset type is indexable
	*
	* @param string	$type_code	the type_code to check
	*
	* @return boolean
	* @access public
	*/
	function isTypeIndexable($type_code)
	{
		$weights = $this->getAssetTypeWeightings($type_code);
		return (isset($weights['indexed']) && $weights['indexed']);

	}//end isTypeIndexable()


	/**
	* Check if the word is indexable
	*
	* Check is performed according to the current indexing parameters
	*
	* @param string		$word	word to check
	*
	* @return boolean
	* @access public
	*/
	function isWordIndexable($word)
	{
		$min_length = $this->attr('min_word_length');
		return $this->isWhiteWord($word) || (strlen($word) >= $min_length && !$this->isNoiseWord($word));

	}//end isWordIndexable()


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
		$this->reindexMetadata($vars['assetid']);

	}//end onIndexMetadata()


	/**
	* Re-indexes the metadata components for the specified asset
	*
	* @param string	$assetid	the assetid to reindex
	*
	* @return void
	* @access public
	*/
	function reindexMetadata($assetid)
	{
		assert_valid_assetid($assetid);
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$fieldids = $mm->getMetadataFields($mm->getSchemas($assetid, true));

		foreach ($fieldids as $fieldid => $field_type) {
			$this->flushIndexableContent($assetid, 'metadata:'.$fieldid);
		}

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$this->addIndexableContent($assetid, $this->getIndexableMetadataContent($asset, $fieldids));

	}//end reindexMetadata()


	/**
	* Returns the indexable metadata content for the specified asset
	*
	* @param &Object	Asset	$asset			The asset to get the indexable content fors
	* @param Array				$fieldids		Array of fieldid => type_code to limit to
	*
	* @return Array
	* @access public
	*/
	function getIndexableMetadataContent(&$asset, $fieldids=Array())
	{
		$index_content = Array();
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$type_code = $asset->type();

		if (is_null($asset)) return $index_content;

		if (empty($fieldids)) {
			$fieldids = $mm->getMetadataFields($mm->getSchemas($asset->id, true));
		}
		$metadata = $mm->getMetadata($asset->id);

		foreach ($fieldids as $fieldid => $field_type) {

			// dont index this fieldid if getMetadataWeighting return false
			if (false === ($weighting = $this->getMetadataWeighting($asset->id, $fieldid))) {
				continue;
			}

			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid, $field_type);

			if (isset($metadata[$field->id]['value'])) {
				$value = $metadata[$field->id]['value'];
			} else {
				$value = $field->attr('default');
			}

			$force_processing = 1; // will tell the function to process values in any case

			// check field_type to define the indexing way
			if ($field_type == 'metadata_field_date') {
				$index_content[] = Array(
									'value'		=> $value,
									'type_code'	=> $type_code,
									'type'		=> 'datetime',
									'component'	=> 'metadata:'.$field->id,
									'score'		=> $weighting,
								   );
			} else {
				switch ($field_type) {
					case 'metadata_field_select':
						$key_type = 'selection';
					break;

					case 'metadata_field_thesaurus':
						$key_type = 'thesaurus';
					break;

					default:
						$key_type = 'text';
				}

				$keywords = retrieve_keywords_replacements($value, '.');
				$value = replace_keywords($value, $mm->generateKeywordReplacements($asset, $keywords, false));

				$word_counts = get_word_counts(strtolower($value));


				foreach ($word_counts as $word => $count) {
					$index_content[] = Array(
										'value'		=> $word,
										'type_code'	=> $type_code,
										'type'		=> $key_type,
										'component'	=> 'metadata:'.$field->id,
										'score'		=> $count * $weighting,
									   );
				}
			}//end if metadata_date
		}//end foreach fieldids

		return $index_content;

	}//end getIndexableMetadataContent()


//--        WEIGHTINGS        --//


	/**
	* Returns the weightings for an assets core components and its attributes
	*
	* The Following presedence rules are followed in order when finding the weightings for
	* a particular asset:
	*
	* <ul>
	*  <li> Any explicit weightings for the asset id are checked </li>
	*  <li> Any weightings for a tree in which the asset resides are checked</li>
	*  <li> Any weightings for a asset type are checked </li>
	*  <li> Any global weightings for all asset types are checked </li>
	*  <li> All components are returned a default weighing of 1</li>
	* </ul>
	*
	* @param string	$assetid 	the assetid to obtain the weightings for
	*
	* @return Array()
	* @access public
	*/
	function getWeightings($assetid)
	{
		// firstly, check if there are any weightings specific to this asset
		$weightings = $this->getAssetWeightings($assetid);

		if (empty($weightings)) {
			// if we can't find any weightings specific to this asset, check
			// to see if there are any tree weightings for this asset
			$weightings = $this->getAssetTreeWeightings($assetid, false);

			// if we can't find any tree weightings that this asset exists under
			// check the asset type weightings.
			if (empty($weightings)) {
				$type_code = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), true, 'type_code');
				$type_code = $type_code[$assetid];
				$weightings = $this->getAssetTypeWeightings($type_code);

				// if there are no type weightings, then we will have to create
				// some default values for this assetid
				if (empty($weightings)) {
					$weightings  = Array();
					$asset_comps = $this->getIndexableAssetComponents();
					$attr_comps  = $this->getIndexableAttributes($type_code);
					$components  = array_merge($asset_comps, $attr_comps);

					foreach (array_keys($components) as $component) {
						$weightings[$component] = 1;
					}
					return $weightings;
				}
			}
		}

		$indexed_weightings = Array();
		// only add the weightings if the individual components are indexed
		foreach (Array('attr_weights', 'asset_weights') as $type) {
			foreach ($weightings[$type] as $component => $info) {
				if ($info['indexed']) {
					$indexed_weightings[$component] = $info['weight'];
				}
			}
		}
		return $indexed_weightings;

	}//end getWeightings()


	/**
	* Returns the metadata weightings for the specified assetid and metadata fieldid
	*
	* The following presedence rules are followed in order when finding the weightings for
	* a particular asset:
	*
	* <ul>
	*  <li> Any explicit weightings for the asset id are checked </li>
	*  <li> Any weightings for a tree in which the asset resides are checked </li>
	*  <li> Any weightings for a asset type are checked </li>
	*  <li> Any global weightings for all asset types are checked </li>
	*  <li> A default weighing of 1 is returned </li>
	* </ul>
	*
	* @param string	$assetid 	the assetid to get the weightings for
	* @param string	$fieldid	the fieldid of the metadata field
	*
	* @return Array() or false if the fieldid should not be indexed
	* @access public
	*/
	function getMetadataWeighting($assetid, $fieldid)
	{
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$section_link = $GLOBALS['SQ_SYSTEM']->am->getLink($fieldid, SQ_LINK_TYPE_2, 'metadata_section', true, null, 'minor');
		$schema_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($section_link['majorid'], SQ_LINK_TYPE_2, 'metadata_schema', true, 'minor');
		$sectionid    = $section_link['majorid'];
		$schemaids    = Array();

		foreach ($schema_links as $schema_info) {
			$schemaids[] = $schema_info['majorid'];
		}

		$info      = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($fieldid), Array(), true, 'type_code');
		$type_code = $info[$fieldid];

		$weight_sources = Array(
							'Asset'		=> $assetid,
							'AssetTree'	=> $assetid,
							'AssetType'	=> $type_code,
						  );

		foreach ($weight_sources as $type => $key) {
			$fn = 'get'.$type.'weightings';
			$weightings = $this->$fn($key);
			if (!empty($weightings) && isset($weightings['metadata_weights'])) {
				$weight = $this->_getLocalMetadataWeightings($weightings['metadata_weights'], $fieldid, $sectionid, $schemaids);
				// if the weight is false then we don't want to index this field
				if ($weight === false) return false;
				// if the weight is not -1 then this is the wieght to use
				// otherwise continue to the next source
				if ($weight != -1) return $weight;
			}
		}

		// return a default weight of 1 for this field
		return 1;

	}//end getMetadataWeighting()


	/**
	* Returns the metadata weightings for a fieldid from the appropriate metadata asset
	*
	* Drills up the metadata tree going from the field => section => schema to find
	* any metadata weightings applicable for this field
	*
	* @param Array		$weightings	the weightigns to search
	* @param string		$fieldid	the fieldid of the metadata field
	* @param string		$sectionid	the sectionid that the field is a child of
	* @param string 	$schemaids	the schemaid that the section is a child of
	*
	* @return int	the weighting for the field or false if the fieldid is not to be indexed
	* 				-1 if no weighting could be found
	* @access private
	*/
	function _getLocalMetadataWeightings($weightings, $fieldid, $sectionid, $schemaids)
	{
		if (!empty($weightings)) {
			// check to see if there is an explicit weighting for the field
			if (isset($weightings[$fieldid])) {
				if (!$weightings[$fieldid]['indexed']) return false;
				return $weightings[$fieldid]['weight'];
			} else {
				// check for a weighting for the section that the field is under
				if (isset($weightings[$sectionid])) {
					if (!$weightings[$sectionid]['indexed']) return false;
					return $weightings[$sectionid]['weight'];
				} else {
					// check all the schemas that the section belongs under
					// its possible that this field is in multiple schemas, so find the
					// hightest weighting from all the schemas that it is under
					$highest_weight = -1;

					// check the index flag on all the schemas, and only turn off indexing
					// for this fieldid if all schemas have indexing off
					$index = false;

					foreach ($schemaids as $schemaid) {
						if (isset($weightings[$schemaid])) {
							if ($weightings[$schemaid]['indexed']) $index = true;
							if ($weightings[$schemaid]['weight'] > $highest_weight) {
								$highest_weight = $weightings[$schemaid]['weight'];
							}
						}
					}
					if (!$index) return false;
					if ($highest_weight != -1) return $highest_weight;

				}
			}
		}
		// no weight could be found
		return -1;

	}//end _getLocalMetadataWeightings()


	/**
	* Returns the Asset Type weightings for the specified asset type, or all
	* asset types if the specified type code is null
	*
	* @param string	$type_code	the type code of the wanted weightings, or null
	*							for all asset type weightings
	*
	* @return Array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getAssetTypeWeightings($type_code=null, $strict_type_code=false)
	{
		$root_folder_assetid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder');
		// type weightings are stored under the root folder tree weightings
		$weights = $this->getAssetTreeWeightings($root_folder_assetid, true, true);

		$type_weights = array_get_index($weights, 'type_weights', Array());
		if (is_null($type_code)) return $type_weights;

		if (!$strict_type_code) {
			$type_hier = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, true);
		} else {
			$type_hier = Array($type_code);
		}
		foreach ($type_hier as $type_code) {
			if (in_array($type_code, array_keys($type_weights))) {
				return $type_weights[$type_code];
			}
		}
		return Array();

	}//end getAssetTypeWeightings()


	/**
	* Returns the Asset Tree weightings for the specified assetid, or all
	* weightings registered for asset trees if the specified assetid is null
	*
	* @param string		$assetid			the assetid of the wanted tree weightings
	*										or null for all asset tree weightings
	*
	* @param boolean	$strict_assetid		if TRUE only weightings that have been
	*										customised explicitly using the specified assetid
	*										will be returned, otherwise the closest weighting
	*										customisation will be returned
	*
	* @param boolean	$strict_type_code	if TRUE only weightings matching the asset type of the
	*										specified assetid will be returned
	*
	* @return Array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getAssetTreeWeightings($assetid=null, $strict_assetid=true, $strict_type_code=false)
	{
		$weightings_file = $this->data_path.'/asset_tree_weightings.inc';
		if (file_exists($weightings_file)) {
			require $weightings_file;

			if (!is_null($assetid)) {

				if ($strict_assetid) {
					if (isset($asset_tree_weightings[$assetid])) {
						return $asset_tree_weightings[$assetid];
					}
				} else {
					$type_code = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), true, 'type_code');
					$type_code = $type_code[$assetid];
					// firstly, check to see if there is an explicit entry for this assetid
					if (isset($asset_tree_weightings[$assetid]['type_weights'][$type_code])) {
						return $asset_tree_weightings[$assetid]['type_weights'][$type_code];
					}

					// there is no explicit weighting customisation for the assetid
					// so do a tree lookup for the closest one. getParents ORDER's by treeid
					// by default so we can be garunteed that the closest assetids are at the top of the array
					$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($assetid);
					$closest_parentid = 0;

					foreach (array_keys($parents) as $parentid) {
						if (in_array($parentid, array_keys($asset_tree_weightings))) {
							// check to see if there are any customisations under this
							// tree for the specified asset type. Note that tree proximity supersedes
							// the type hierachy, so search the type hierachy if applicable
							if (!$strict_type_code) {
								$type_hier = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, true);
							} else {
								$type_hier = Array($type_code);
							}
							foreach ($type_hier as $type_code) {
								if (in_array($type_code, array_keys($asset_tree_weightings[$parentid]['type_weights']))) {
									$closest_parentid = $parentid;
									break;
								}
							}
						}
					}
					if ($closest_parentid != 0) {
						return $asset_tree_weightings[$closest_parentid]['type_weights'][$type_code];
					}
				}//end if strict_assetid
			} else {
				return $asset_tree_weightings;
			}//end if !is_null(assetid)
		}//end if file_exists

		return Array();

	}//end getAssetTreeWeightings()


	/**
	* Saves the asset tree weightings to file
	*
	* @param Array	$asset_tree_weightings	the complete asset tree weightings to save
	*
	* @return boolean
	* @access public
	*/
	function saveAssetTreeWeightings($asset_tree_weightings)
	{
		return $this->_saveWeightings($asset_tree_weightings, 'asset_tree');

	}//end saveAssetTreeWeightings()


	/**
	* Returns the Asset Type weightings
	*
	* @param string	$assetid	the assetid of the wanted weightings
	*							if null, weightings for all assets set will be returned
	*
	* @return Array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getAssetWeightings($assetid=null)
	{
		$weightings_file = $this->data_path.'/asset_weightings.inc';
		if (file_exists($weightings_file)) {
			require $weightings_file;
			if (!is_null($assetid)) {
				if (isset($asset_weightings[$assetid])) {
					return $asset_weightings[$assetid];
				}
			} else {
				return $asset_weightings;
			}
		}
		return Array();

	}//end getAssetWeightings()


	/**
	* Saves the Asset Weightings
	*
	* @param array	$type_weights	the array of weights to save
	*
	* @return boolean
	* @access public
	* @see getTypeWeightings
	*/
	function saveAssetWeightings($asset_weightings)
	{
		return $this->_saveWeightings($asset_weightings, 'asset');

	}//end saveAssetWeightings()


	/**
	* Returns the Asset Type weightings
	*
	* @param string	$assetid	the assetid of the wanted weightings
	*							if null, weightings for all assets set will be returned
	*
	* @return Array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getMetadataWeightings($assetid=null)
	{
		$weightings_file = $this->data_path.'/metadata_weightings.inc';
		if (file_exists($weightings_file)) {
			require $weightings_file;
			if (!is_null($assetid)) {
				if (isset($metadata_weightings[$assetid])) {
					return $metadata_weightings[$assetid];
				}
			} else {
				return $metadata_weightings;
			}
		}
		return Array();

	}//end getMetadataWeightings()


	/**
	* Saves the metadata weightings to file
	*
	* @param $metadata_weightings	the complete metadata weightings to save
	*
	* @return boolean
	* @access public
	*/
	function saveMetadataWeightings($metadata_weightings)
	{
		return $this->_saveWeightings($metadata_weightings, 'metadata');

	}//end saveMetadataWeightings()


	/**
	* Saves the weightings to the specified file
	*
	* @param array	$weightings the weightings to save
	* @param string	$type		the type of weightings, which should be one of
	*							asset_type, asset, asset_tree
	*
	* @return boolean
	* @access private
	*/
	function _saveWeightings($weightings, $type)
	{
		if (!in_array($type, Array('asset_type', 'asset', 'asset_tree', 'metadata'))) {
			trigger_error('Invalid weightings type "'.$type.'"', E_USER_ERROR);
			return false;
		}
		$weightings_file = $this->data_path.'/'.$type.'_weightings.inc';

		$output = '<'.'?php'."\n".' $'.$type.'_weightings = ';
		$output .= var_export($weightings, true);
		$output .= "\n?".'>';

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		create_directory($this->data_path);
		if (!string_to_file($output, $weightings_file)) {
			trigger_error('Could not write out the '.str_replace('_', ' ', $type).' weightings to file', E_USER_WARNING);
			return false;
		}
		return true;

	}//end _saveWeightings()


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

		$sql = 'SELECT
					LOWER(value) as keyword, SUM(score) as total_score
				FROM
					sq_sch_idx
				WHERE
					assetid = '.$db->quoteSmart($asset->id).
				  ($include_metadata ? '' : ' AND component NOT LIKE '.$db->quoteSmart('metadata:%')).'
				GROUP BY
					keyword
				ORDER BY
					total_score DESC';

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
	* Generates a word list from the search string
	*
	* @param string	$words	the words string
	*
	* @return Array()
	* @access public
	*/
	function generateWordList($words)
	{
		if (!is_array($words)) {
			$words = split(' +', strtolower($words));
			foreach (array_keys($words) as $i) {
				// convert wildcard symbols to corresponding SQL wildcards
				$words[$i] = strtr($words[$i], '*?', '%_');

				// if this is an email, skip the following check...
				$at_pos  = strpos($words[$i], '@');
				$dot_pos = strpos($words[$i], '.');
				if ($at_pos === false && $dot_pos === false) {
					// preserve the wildcard symbols - i.e. don't quote them
					if (ereg('^[A-z%_0-9]+$', $words[$i])) {
							$words[$i] = addslashes($words[$i]);
					} else {
						unset($words[$i]);
					}
				}
			}
		}
		return $words;

	}//end generateWordList()


	/**
	* Processes a search request
	*
	* Formulates search query and returns the results
	*
	* Each search request is logged to the 'search' log in the format:
	* <PRE>
	* Array('terms'   => array (
	*						first term => count of result assets for this term only
	*						second term => ...
	*						),
	*		'results' => total number of result assets for this request,
	*		'assetid' => id of the asset requesting this search
	* );
	* </PRE>
	* This array will be in the 'data' section of the log entry (to be accessed via the log manager)
	*
	* @param array	$search_info	an array of search settings in the form
	* <PRE>
	* Array('root_assetid'    => asset ID|Array(asset IDs) - asset IDs to search under,
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
	*       'assetid'           => asset ID of asset requesting the search,
	* );
	*</PRE>
	*
	* @return array
	* @access public
	*/
	function processSearch($search_info, $include_context=false)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$total_asset_scores = null;

		$group_by =  'GROUP BY ai.assetid';
		$group_by .= $include_context ? ', ai.component' : '';

		  /////////////////////////////
		 //  CONSTRUCT CACHING KEY  //
		/////////////////////////////

		$search_cache_key = is_array($search_info['root_assetid'])?implode(',',$search_info['root_assetid']):$search_info['root_assetid'];
		if (isset($search_info['search_statuses'])) {
			$search_cache_key .= implode(':', $search_info['search_statuses']);
		}
		if (isset($search_info['search_types'])) {
			$search_cache_key .= implode(':', $search_info['search_types']);
		}

		if (!empty($search_info['sort_by'])) {
			if ($search_info['sort_by']['type'] == 'field') {
				$key = $search_info['sort_by']['params']['field'];
				$group_by .= ', a.'.$key;
			} else if ($search_info['sort_by']['type'] == 'asset_attrib') {
				$key = $search_info['sort_by']['params']['type_code'].$search_info['sort_by']['params']['attribute'];
			}
			$search_cache_key .= $key;
		}

		// we need the cache manager to cache the results of queries
		$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

		  ///////////////////////////////////
		 //  CONSTRUCT BASE SEARCH QUERY  //
		///////////////////////////////////
		$start_sql_array = $this->constructBaseSearchQuery($search_info);
		$start_sql_array['select'] .= $include_context ? ', ai.component as source' : '';

		$start_sql = implode(' ', $start_sql_array);

		  ///////////////////////////////
		 //  SEARCH ASSET ATTRIBUTES  //
		///////////////////////////////

		if (isset($search_info['asset_attrib'])) {

			foreach ($search_info['asset_attrib'] as $attr_name => $attr_info) {

				// get a list of words from the search terms
				$attr_info['words'] = $this->generateWordList($attr_info['words']);

				if ($attr_info['params']['asset_type'] == '') continue;
				$attribute = &$GLOBALS['SQ_SYSTEM']->am->getAttribute($attr_info['params']['attrid']);
				if (is_null($attribute)) continue;

				$field_asset_scores = null;

				switch ($attribute->type()) {
					case 'datetime' :
						$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
						$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

						$where = '';
						if ($from != '---------- --:--:--') {
							$where .= ' AND ai.value >= '.$db->quote($from);
						}
						if ($to != '---------- --:--:--') {
							$where .= ' AND ai.value <= '.$db->quote($to);
						}
						if (empty($where)) continue;

						$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$from.'_'.$to);
						if (empty($result)) {
							$sql = $start_sql.'
									AND ai.type = '.$db->quote('datetime').'
									AND ai.component = '.$db->quote('attr:'.$attr_info['params']['asset_type'].':'.$attribute->name).' '.$where.' '.$group_by;

							$result = $db->getAssoc($sql);
							assert_valid_db_result($result);

							$cm->saveToCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$from.'_'.$to, serialize($result));
						} else {
							$result = unserialize($result);
						}

						$field_asset_scores = $result;
					break;

					default :
						$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
						if ($field_search_logic != 'AND' && $field_search_logic != 'OR') {
							$field_search_logic = 'AND';
						}

						foreach ($attr_info['words'] as $word) {
							$this->_tmp['terms_used'][$word] = $word;
							if (!$this->isWordIndexable($word)) continue;

							$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$word);
							if (empty($result)) {
								$sql = $start_sql.'
									AND ai.component = '.$db->quote('attr:'.$attr_info['params']['asset_type'].':'.$attribute->name).' AND (ai.value LIKE '.$db->quote('%'.$word.'%').') '.$group_by;

								$result = $db->getAssoc($sql);
								assert_valid_db_result($result);

								$cm->saveToCache($this->id, $this->type(), $search_cache_key.'asset_attrib'.$word, serialize($result));
							} else {
								$result = unserialize($result);
							}

							$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic, $word);
						}
					break;
				}//end switch attribute type

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  //////////////////////////
		 //  SEARCH ASSET DATES  //
		//////////////////////////
		if (isset($search_info['asset_date'])) {

			$valid_dates = Array('created', 'updated', 'published');

			foreach ($search_info['asset_date'] as $attr_name => $attr_info) {

				// get a list of words from the search terms
				$attr_info['words'] = $this->generateWordList($attr_info['words']);

				if (!isset($attr_info['params']['date_type'])) {
					continue;
				}
				if (!in_array($attr_info['params']['date_type'], $valid_dates)) {
					continue;
				}

				$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
				$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

				$where = '';
				if ($from != '---------- --:--:--') {
					$where .= ' AND ai.value >= '.$db->quote($from);
				}
				if ($to != '---------- --:--:--') {
					$where .= ' AND ai.value <= '.$db->quote($to);
				}
				if (empty($where)) continue;

				$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'asset_date'.$from.'_'.$to);
				if (empty($result)) {
					$sql = $start_sql.'
							AND ai.type = '.$db->quote('datetime').'
							AND ai.component = '.$db->quote('__'.$attr_info['params']['date_type'].'__').'
							'.$where.' '.$group_by;

					$result = $db->getAssoc($sql);
					assert_valid_db_result($result);

					$cm->saveToCache($this->id, $this->type(), $search_cache_key.'asset_date'.$from.'_'.$to, serialize($result));
				} else {
					$result = unserialize($result);
				}

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $result, $search_info['logic']);
			}
		}
		  /////////////////////////////////
		 //  SEARCH INCLUDE ALL FIELDS  //
		/////////////////////////////////
		if (isset($search_info['include_all'])) {

			foreach ($search_info['include_all'] as $attr_name => $attr_info) {

				// get a list of words from the search terms
				$attr_info['words'] = $this->generateWordList($attr_info['words']);

				$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
				if ($field_search_logic != 'AND' && $field_search_logic != 'OR') {
					$field_search_logic = 'AND';
				}

				$field_asset_scores = null;

				foreach ($attr_info['words'] as $word) {
					$this->_tmp['terms_used'][$word] = $word;

					if (!$this->isWordIndexable($word)) continue;

					$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'include_all'.$word);
					assert_valid_db_result($result);
					if (empty($result)) {
						$sql = $start_sql.' AND (ai.value LIKE '.$db->quote('%'.$word.'%').') '.$group_by;

						if ($include_context) {
							$result = $db->getAll($sql);
						} else {
							$result = $db->getAssoc($sql);
						}

						assert_valid_db_result($result);
						$cm->saveToCache($this->id, $this->type(), $search_cache_key.'include_all'.$word, serialize($result));
					} else {
						$result = unserialize($result);
					}

					$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic, $word);
				}

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  ///////////////////////
		 //  SEARCH METADATA  //
		///////////////////////
		if (isset($search_info['metadata'])) {

			foreach ($search_info['metadata'] as $attr_name => $attr_info) {

				// get a list of words from the search terms
				$attr_info['words'] = $this->generateWordList($attr_info['words']);

				if ($attr_info['params']['assetid'] <= 0) continue;
				$metadata_field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($attr_info['params']['assetid']);
				if (is_null($metadata_field)) continue;

				$field_asset_scores = null;

				$field_type = get_class($metadata_field);
				switch ($field_type) {
					case 'metadata_field_date' :
						$from = array_get_index($attr_info['words'], 'from', '---------- --:--:--');
						$to   = array_get_index($attr_info['words'], 'to', '---------- --:--:--');

						$where = '';
						if ($from != '---------- --:--:--') {
							$where .= ' AND ai.value >= '.$db->quote($from);
						}
						if ($to != '---------- --:--:--') {
							$where .= ' AND ai.value <= '.$db->quote($to);
						}
						if (empty($where)) continue;

						$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'metadata'.$from.'_'.$to);
						if (empty($result)) {
							$sql = $start_sql.'
									AND ai.type = '.$db->quote('datetime').'
									AND ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['assetid']).'
									'.$where.' '.$group_by;

							$result = $db->getAssoc($sql);
							assert_valid_db_result($result);

							$cm->saveToCache($this->id, $this->type(), $search_cache_key.'metadata'.$from.'_'.$to, serialize($result));
						} else {
							$result = unserialize($result);
						}

						$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, 'AND');
					break;

					default :
						switch ($field_type) {
							case 'metadata_field_select':
								$key_type = 'selection';
							break;

							case 'metadata_field_thesaurus':
								$key_type = 'thesaurus';
							break;

							default:
								$key_type = 'text';
						}

						$field_search_logic = array_get_index($attr_info, 'search_logic', 'AND');
						if ($field_search_logic != 'AND' && $field_search_logic != 'OR') {
							$field_search_logic = 'AND';
						}

						foreach ($attr_info['words'] as $word) {
							$this->_tmp['terms_used'][$word] = $word;

							$result = $cm->loadFromCache($this->id, $this->type(), $search_cache_key.'metadata'.$word);
							if (empty($result)) {
								$sql = $start_sql.'
										AND ai.type = '.$db->quote($key_type).'
										AND ai.component = '.$db->quote('metadata:'.(int)$attr_info['params']['assetid']).'
										AND ai.value LIKE '.$db->quote('%'.$word.'%').' '.$group_by;

								$result = $db->getAssoc($sql);
								assert_valid_db_result($result);
								$cm->saveToCache($this->id, $this->type(), $search_cache_key.'metadata'.$word, serialize($result));
							} else {
								$result = unserialize($result);
							}

							$field_asset_scores = $this->combineAssetScores($field_asset_scores, $result, $field_search_logic, $word);
						}//end foreach word
					break;
				}

				$total_asset_scores = $this->combineAssetScores($total_asset_scores, $field_asset_scores, $search_info['logic']);
			}
		}


		  /////////////////////////////
		 //  SEARCH EXCLUDE FIELDS  //
		/////////////////////////////
		if (isset($search_info['exclude'])) {

			foreach ($search_info['exclude'] as $attr_name => $attr_info) {

				// get a list of words from the search terms
				$attr_info['words'] = $this->generateWordList($attr_info['words']);

				$sql = $start_sql.'
					AND (ai.value LIKE \'%'.implode("%' OR ai.value LIKE '%", $attr_info['words']).'%\') '.$group_by;

				$result = $db->getAssoc($sql);
				assert_valid_db_result($result);

				foreach (array_keys($result) as $exclude_assetid) {
					if (isset($total_asset_scores[$exclude_assetid])) {
						unset($total_asset_scores[$exclude_assetid]);
						unset($this->tmp_['term_totals'][$exclude_assetid]);
					}
				}
			}
		}

		if (is_null($total_asset_scores)) {
			$total_asset_scores = Array();
		} else if ($include_context) {
			return $total_asset_scores;
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
			if (isset($search_info['sort_descending']) && !$search_info['sort_descending']) {
				ksort($sorted_scores);
			} else {
				krsort($sorted_scores);
			}
			$total_asset_scores = Array();
			foreach ($sorted_scores as $sort_by => $results) {
				foreach ($results as $assetid => $search_score) {
					$total_asset_scores[$assetid] = $search_score;
				}
			}
		}

		$log_contents['terms']   = $this->_getTermTotals();
		$log_contents['results'] = count($total_asset_scores);
		$log_contents['assetid'] = $search_info['assetid'];

		log_write($log_contents, 'search');

		return $total_asset_scores;

	}//end processSearch()


	/**
	* Processes a basic search request without search terms
	*
	* Accepts the same input array as processSearch but does not use any search terms
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
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$search_sql = 'SELECT DISTINCT(a.assetid), 1 AS score
					   FROM
							sq_ast_lnk_tree alt1,
							sq_ast_lnk_tree alt2,
							sq_ast_lnk al1,
							sq_ast_lnk al2,
							sq_ast a';
		if ($user_restrictions) {
			$search_sql .= ', sq_ast_perm ap ';
		}

		$search_sql .= ' WHERE ';

		if ($user_restrictions) {
			$search_sql .= 'a.assetid = ap.assetid AND ';
		}

		if (!is_array($search_info['root_assetid'])){
			$search_info['root_assetid'] = Array($search_info['root_assetid']);
		}

		$roots = Array();
		foreach ($search_info['root_assetid'] as $root_id) {
			$roots[] = $db->quote($root_id);
		}

		$roots = implode(',', $roots);

		$search_sql .= ' al1.majorid IN ('.$roots.')
						 AND al1.linkid  = alt1.linkid
						 AND alt2.treeid LIKE (alt1.treeid || '.$db->quote('%').')
						 AND alt2.linkid = al2.linkid
						 AND a.assetid  = al2.minorid';

		// add in status restrictions
		if (isset($search_info['search_statuses']) && !empty($search_info['search_statuses'])) {
			$statues = $search_info['search_statuses'];
			for ($i = 0; $i < count($statues); $i++) {
				$statues[$i] = $db->quote($statues[$i]);
			}
			$search_sql .= ' AND a.status IN ('.implode(', ', $statues).')';
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$search_sql .= ' AND a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		if (!empty($search_info['search_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['search_types']); NULL !== ($i = key($search_info['search_types'])); next($search_info['search_types'])) {
				if ($search_info['search_types'][$i]) {
					$inherited_types[] = $db->quote($i);
				} else {
					$normal_types[] = $db->quote($i);
				}
			}

			$type_code_cond = Array();

			if (!empty($inherited_types)) {
				$type_code_cond[] = 'inhd_type_code IN ('.implode(', ', $inherited_types).')';
			}

			if (!empty($normal_types)) {
				$type_code_cond[] = 'type_code IN ('.implode(', ', $normal_types).')';
			}

			$type_code_cond = implode(' OR ', $type_code_cond);


			$search_sql .= ' AND a.type_code IN (
												SELECT type_code
												FROM sq_ast_typ_inhd
												WHERE '.$type_code_cond.'
												)';
		}

		$userid_cond = '';
		if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
			array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quote($userids[$i]);
			}
			$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
		}

		if (!empty($userid_cond)) {
			$search_sql .= $userid_cond;
			$search_sql .= ' AND ap.permission >= '.$db->quote(SQ_PERMISSION_READ).'
							 AND ap.granted = 1';
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
	* Counts the number of result asset ids each term relates to
	*
	* @access private
	* @return void
	*/
	function _getTermTotals()
	{
		$terms_used  = array_get_index($this->_tmp, 'terms_used', Array());
		$term_totals = array_get_index($this->_tmp, 'term_totals', Array());
		if (empty($terms_used)) return Array();

		$final_term_totals = Array();
		foreach ($terms_used as $term) {
			$final_term_totals[$term] = 0;
		}

		foreach ($term_totals as $asset_id => $terms) {
			foreach ($terms as $term) {
				$final_term_totals[$term]++;
			}
		}

		return $final_term_totals;

	}//end _getTermTotals()


	/**
	* Checks a word against a thesaurus asset for synonyms
	*
	* @param string				$word		the word being checked
	* @param object Thesaurus	$thesaurus	thesaurus asset being used
	*
	* @return Array(string)	the list of suggestions
	* @access public
	*/
	function findSynonymsForWord($word, &$thesaurus)
	{
		if (!is_a($thesaurus, 'thesaurus')) {
			// why did you pass us a lemon instead of a thesaurus?
			trigger_localised_error('SCH0008', E_USER_WARNING, $thesaurus->getName(), $thesaurus->id);
			return Array();
		}

		$entity = $thesaurus->getEntityByName($word);

		if (empty($entity)) return Array();

		$relation = $thesaurus->getRelationByName('Synonyms');

		return $thesaurus->getChildEntitiesInRelation($relation['id'], $entity['id']);

	}//end findSynonymsForWord()


	/**
	* Spell-checks a word and returns a list of suggestions if available
	*
	* This function checks for the existence of the pSpell extension. If it does
	* not exist (eg. Windows server), or an error otherwise occurs in the loading
	* of the pSpell instance, we will proceed as if no suggestions are available.
	*
	* @param string	$word		the word to be checked
	* @param string $language	the language to be used to spell-check
	*
	* @return Array(string)	the list of suggestions, one considered most relevant
	*						by pSpell in index zero, and so on
	* @access public
	*/
	function spellCheckWord($word, $language='en')
	{
		if (!$this->spellCheckAvailable()) {
			// spell check not available, bugger
			return Array();
		}

		$pspell_config = pspell_config_create($language);
		pspell_config_mode($pspell_config, PSPELL_FAST);
		$pspell_instance = pspell_new_config($pspell_config);

		if (!$pspell_instance) return Array();

		$check_spelling = true;

		// ignore numbers
		if ($check_spelling) {
			$converted_word = (int)$word;
			if ((string)$converted_word == $word) {
				$check_spelling = false;
			}
		}

		// ignore words in all uppercase
		if ($check_spelling) {
			$converted_word = strtoupper($word);
			if ($converted_word == $word) $check_spelling = false;
		}

		// if we aren't checking spelling OR word is correct, return no suggestions
		if (!$check_spelling || pspell_check($pspell_instance, $word)) {
			return Array();
		} else {
			// word incorrect, if any suggestions then return them
			$suggestions = pspell_suggest($pspell_instance, $word);
			return $suggestions;
		}

	}//end spellCheckWord()


	/**
	* Returns true if the spell checker is available
	*
	* Written as a wrapper function so that if the spell-checker changes for some
	* reason, then this can be updated. It currently just checks whether the
	* pSpell extension is available.
	*
	* @return boolean
	* @access public
	*/
	function spellCheckAvailable()
	{
		return extension_loaded('pspell');

	}//end spellCheckAvailable()


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

		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		// sorting by relevance by default
		$sort_string = 'SUM(ai.score)';

		if (!empty($search_info['sort_by'])) {
			if ($search_info['sort_by']['type'] == 'field') {
				$sort_by = $search_info['sort_by']['params']['field'];
				$valid_sort_fields = Array('assetid', 'name', 'short_name', 'created', 'updated', 'published', 'type_code');
				if (in_array($sort_by, $valid_sort_fields)) {
					// sorting by a field in the asset table
					$sort_string = 'a.'.$sort_by;
				}
			}
		}

		$sql = Array();

		$sql['select'] = 'SELECT ai.assetid, SUM(ai.score) as search_score, '.$sort_string.' AS sort_by';

		$sql['from']   = 'FROM
							sq_sch_idx ai,
							sq_ast_lnk_tree alt1,
							sq_ast_lnk_tree alt2,
							sq_ast_lnk al1,
							sq_ast_lnk al2,
							sq_ast a';
		if ($user_restrictions) {
			$sql['from'] .= ', sq_ast_perm ap ';
		}

		$sql['where'] = 'WHERE ';

		if ($user_restrictions) {
			$sql['where'] .= 'a.assetid = ap.assetid AND ';
		}

		if (!is_array($search_info['root_assetid'])){
			$search_info['root_assetid'] = Array($search_info['root_assetid']);
		}

		$roots = Array();
		foreach ($search_info['root_assetid'] as $root_id) {
			$roots[] = $db->quote($root_id);
		}

		$roots = implode(',', $roots);

		$sql['where'] .= '  a.assetid = ai.assetid
							AND al1.majorid IN ('.$roots.')
							AND al1.linkid  = alt1.linkid
							AND alt2.treeid LIKE (alt1.treeid || '.$db->quote('%').')
							AND alt2.linkid = al2.linkid
							AND ai.assetid  = al2.minorid';

		// add in status restrictions
		if (isset($search_info['search_statuses']) && !empty($search_info['search_statuses'])) {
			$statues = $search_info['search_statuses'];
			for ($i = 0; $i < count($statues); $i++) {
				$statues[$i] = $db->quote($statues[$i]);
			}
			$sql['where'] .= ' AND a.status IN ('.implode(', ', $statues).')';
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$sql['where'] .= ' AND a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		if (!empty($search_info['search_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['search_types']); NULL !== ($i = key($search_info['search_types'])); next($search_info['search_types'])) {
				if ($search_info['search_types'][$i]) {
					$inherited_types[] = $db->quote($i);
				} else {
					$normal_types[] = $db->quote($i);
				}
			}

			$type_code_cond = Array();

			if (!empty($inherited_types)) {
				$type_code_cond[] = 'inhd_type_code IN ('.implode(', ', $inherited_types).')';
			}

			if (!empty($normal_types)) {
				$type_code_cond[] = 'type_code IN ('.implode(', ', $normal_types).')';
			}

			$type_code_cond = implode(' OR ', $type_code_cond);

			$sql['where'] .= 'AND ai.type_code IN (
									SELECT type_code
									FROM sq_ast_typ_inhd
									WHERE '.$type_code_cond.'
								)';
		}

		$userid_cond = '';
		if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false));
			array_push($userids, 0, (int) $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quote($userids[$i]);
			}
			$userid_cond = ' AND ap.userid  IN ('.implode(',', $userids).')';
		}

		if (!empty($userid_cond)) {
			$sql['where'] .= $userid_cond;
			$sql['where'] .= ' AND ap.permission >= '.$db->quote(SQ_PERMISSION_READ).'
							   AND ap.granted = 1';
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
	function combineAssetScores($asset_scores1, $asset_scores2, $logic='AND', $word=null)
	{
		$result_scores = null;
		if (!is_array($asset_scores1) && !is_array($asset_scores2)) {
			$result_scores = Array();
		} else if (!is_array($asset_scores2)) {
			$result_scores = $asset_scores1;
		} else if (!is_array($asset_scores1)) {
			$result_scores = $asset_scores2;
		}

		if (!is_null($result_scores)) {
			if (is_array($result_scores) && !empty($result_scores) && !empty($word)) {
				foreach ($result_scores as $assetid => $data) {
					$this->_tmp['term_totals'][$assetid][$word] = $word;
				}
			}
			return $result_scores;
		}

		// find the intersection/union (depending on logic) of the assetids in field_asset_scores and results
		$assets1 = array_keys($asset_scores1);
		$assets2 = array_keys($asset_scores2);

		$out_asset_scores = $asset_scores1;

		if ($logic == 'AND') {
			$common_assets = array_intersect($assets1, $assets2);

			// get the assets that are to be removed in field_asset_scores
			$removed_assets = array_diff($assets1, $assets2);
			foreach ($removed_assets as $assetid) {
				unset($out_asset_scores[$assetid]);
				if (isset($this->_tmp['term_totals'][$assetid])) {
					unset($this->_tmp['term_totals'][$assetid]);
				}
			}

			foreach ($common_assets as $assetid) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid]['search_score'] += $asset_scores2[$assetid]['search_score'];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}

				if (!empty($word)) {
					$this->_tmp['term_totals'][$assetid][$word] = $word;
				}

			}

		} else if ($logic == 'OR') {

			foreach ($asset_scores2 as $assetid => $data) {
				if (isset($out_asset_scores[$assetid])) {
					$out_asset_scores[$assetid]['search_score'] += $asset_scores2[$assetid]['search_score'];
				} else {
					$out_asset_scores[$assetid] = $asset_scores2[$assetid];
				}

				if (!empty($word)) {
					$this->_tmp['term_totals'][$assetid][$word] = $word;
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
