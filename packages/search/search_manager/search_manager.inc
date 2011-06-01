<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: search_manager.inc,v 1.131.2.6 2007/01/17 21:34:37 lwright Exp $
*
*/

check_ssv_licence();
require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types.
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.131.2.6 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager extends Asset
{
	var $standard_date_fields = NULL;
	var $standard_text_fields = NULL;


	/**
	* Database plugin to handle abstraction of queries to various database types
	* (to better handle different query formats for Oracle and PostgreSQL)
	* @var object
	* @see _getDBPlugin()
	*/
	var $_db_plugin = NULL;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Manager($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);
		$this->standard_date_fields = Array(
										'created'			=> translate('asset_created_date'),
										'updated'			=> translate('asset_updated_date'),
										'published'			=> translate('asset_published_date'),
										'status_changed'	=> translate('asset_status_changed_date'),
									  );
		$this->standard_text_fields = Array(
										'assetid'		=> translate('asset_id'),
										'name'			=> translate('asset_name'),
										'short_name'	=> translate('asset_name_short'),
										'contents'		=> translate('asset_contents'),
									  );

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
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
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a search manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


//--        EVENT STUBS        --//


	/**
	* Event fired when an asset is updated
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onAssetUpdate(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$this->reindexAsset($broadcaster, $vars);

		return TRUE;

	}//end onAssetUpdate()


	/**
	* Event fired when an asset is created
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onAssetCreate(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$this->reindexAsset($broadcaster, $vars);
		$this->reindexAttributes($broadcaster, $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($broadcaster->type(), Array('name')));

		return TRUE;

	}//end onAssetCreate()


	/**
	* Event fired when an asset has its status changed
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onAssetStatusUpdate(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$this->reindexAsset($broadcaster, Array('status_changed', 'published'));

		return TRUE;

	}//end onAssetStatusUpdate()


	/**
	* Event fired when an assets atrtributes have changed
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			a list of attributes that have changed
	*
	* @return boolean
	* @access public
	*/
	function onAttributeChange(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$this->reindexAttributes($broadcaster, $vars);

		return TRUE;

	}//end onAttributeChange()


	/**
	* Event fired when an asset is created
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return boolean
	* @access public
	*/
	function onContentsUpdated(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		$this->reindexContents($broadcaster, $vars);

		return TRUE;

	}//end onContentsUpdated()


	/**
	* Event fired when an asset gets deleted. We want to flush any key data in the index table
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Component is set as $vars['component']
	*
	* @return void
	* @access public
	*/
	function onAssetDeleted(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		return $this->flushIndexableContent($broadcaster->id);

	}//end onAssetDeleted()


//--        ASSET INDEXING        --//


	/**
	* Reindex this asset and, if specified, the metadata associated with this asset
	*
	* @param object	&$asset	the asset to reindex
	* @param array	$vars	the components to reindex - 'asset' to
	*						reindex the core components of the asset, and
	*						'metadata' to reindex metadata or 'all' for both
	*
	* @return void
	* @access public
	*/
	function reindexAsset(&$asset, $vars=Array())
	{
		// if search manager is reindexed then there is infinite recursion!!!
		if ($asset->id == $this->id) return FALSE;

		// only delete the vars that have changed
		$flush_vars = Array();
		if (!in_array('all', $vars)) {
			foreach ($vars as $var) {
				$flush_vars[] = '__'.$var.'__';
			}
		} else {
			// flush all the assets vars
			$flush_vars = array_keys($this->getIndexableAssetComponents());
		}
		if (!empty($flush_vars)) {
			$this->flushIndexableContent($asset->id, $flush_vars);
		}

		// check that we want to index this asset first
		if (!$this->isAssetIndexable($asset->id)) return;

		$index_content = $this->getIndexableAssetContent($asset, $vars);
		$this->addIndexableContent($asset->id, $index_content);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($assetid);


	}//end reindexAsset()


	/**
	* Reindex the specified asset's attributes
	*
	* @param object	&$asset	the asset to reindex
	* @param array	$vars	a list of attribute names that have
	*						changed
	*
	* @return void
	* @access public
	*/
	function reindexAttributes(&$asset, $vars=Array())
	{
		// if search manager is reindexed then there is infinite recursion!!!
		if ($asset->id == $this->id) return FALSE;

		// only delete the attributes that have changed
		$flush_vars = Array();
		if (!in_array('all', $vars)) {
			$flush_vars = $vars;
		} else {
			// flush all the indexable attributes for this asset type
			$flush_vars = array_keys($this->getIndexableAttributes($asset->type()));
		}

		for (reset($flush_vars); NULL !== ($key = key($flush_vars)); next($flush_vars)) {
			$flush_vars[$key] = 'attr:'.$flush_vars[$key];
		}
		if (!empty($flush_vars)) {
			$this->flushIndexableContent($asset->id, $flush_vars);
		}

		// check that we want to index this asset first
		if (!$this->isAssetIndexable($asset->id)) return;

		$index_content = $this->getIndexableAttributeContent($asset, $vars);
		$this->addIndexableContent($asset->id, $index_content);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end reindexAttributes()


	/**
	* Reindex the specified asset's attributes
	*
	* @param object	&$asset	the asset to reindex
	* @param array	$vars	a list of attribute names that have
	*						changed
	*
	* @return void
	* @access public
	*/
	function reindexContents(&$asset, $vars=Array())
	{
		// Find the asset that we want to index the content under.
		// this will be the top most dependant asset above this asset
		if (!$indexing_assetid = $this->getIndexingAssetid($asset)) {
			return FALSE;
		}
		$indexing_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($indexing_assetid);
		$index_content  = $this->getIndexableContent($indexing_asset);

		$this->flushIndexableContent($indexing_assetid, Array('__contents__'));

		// check that we want to index this asset first
		if (!$this->isAssetIndexable($indexing_assetid)) {
			return;
		}

		$this->addIndexableContent($indexing_assetid, $index_content);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($indexing_asset);

	}//end reindexContents()


	/**
	* Returns the assetid in which content should be indexed under for the specified asset
	*
	* @param object	&$asset	the asset to find the indexing asset for
	*
	* @return mixed int|boolean
	* @access public
	*/
	function getIndexingAssetid(&$asset)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, '', FALSE, 'minor');
		if (empty($links)) return FALSE;

		$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id);
		$parents = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($parents, Array(), FALSE, 'type_code');

		if (empty($parents)) return $asset->id;

		// the first dependant parent will be the first element on the array
		$parent = array_shift(array_keys($parents));

		return $parent;

	}//end getIndexingAssetid()


	/**
	* Returns the indexable standard field content for the specified asset
	*
	* @param object	&$asset			the asset to obtain the indexable content for
	* @param array	$changed_vars	the asset vars that have changed
	*
	* @return array
	* @access public
	*/
	function getIndexableAssetContent(&$asset, $changed_vars)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$plugin        =& $this->_getDBPlugin();
		$weightings    =  $this->getWeightings($asset->id);
		$type_code     =  $asset->type();
		$index_content =  Array();
		$reindex_all   =  in_array('all', $changed_vars);
		$indexed_components = $this->getIndexedComponents($asset->id);

		// index the name and the short name
		foreach (Array('name', 'short_name') as $name_type) {

			if (!in_array($name_type, $changed_vars) && !$reindex_all) {
				continue;
			}

			$name_index = '__'.$name_type.'__';
			// if there are no weightings, we dont want to index this name
			if (!isset($weightings[$name_index])) continue;

			$name = ($name_type == 'name') ? strtolower($asset->name) : strtolower($asset->short_name);
			$index_content = array_merge($index_content, $plugin->splitIndexableContent($name, $type_code, 'text', $name_index, $weightings[$name_index]));
		}

		$dates = Array(
					'created'	=> $asset->created,
					'updated'	=> $asset->updated,
				 );
		if (!is_null($asset->published)) {
			$dates['published'] = $asset->published;
		}
		if (!is_null($asset->status_changed)) {
			$dates['status_changed'] = $asset->status_changed;
		}

		foreach ($dates as $type => $date) {

			if (!in_array($type, $changed_vars)  && !$reindex_all) {
				continue;
			}

			// if there are no weightings, we don't want to index this date
			if (!isset($weightings['__'.$type.'__'])) continue;

			$index_content[] = Array(
								'value'		=> ts_iso8601($date),
								'type_code'	=> $type_code,
								'type'		=> 'datetime',
								'component'	=> '__'.$type.'__',
								'score'		=> $weightings['__'.$type.'__'],
							   );
		}

		if (isset($weightings['__assetid__'])) {

			if (in_array('__assetid__', $changed_vars) || $reindex_all) {

				$index_content[] = Array(
									'value'		=> $asset->id,
									'type_code'	=> $type_code,
									'type'		=> 'int',
									'component'	=> '__assetid__',
									'score'		=> $weightings['__assetid__'],
								   );
			}
		}

		return $index_content;

	}//end getIndexableAssetContent()


	/**
	* Returns the indexable content for the specified asset's attributes
	*
	* @param object	&$asset			the asset to obtain the indexable content for
	* @param array	$changed_vars	attributes that have been changed
	*
	* @return array
	* @access public
	*/
	function getIndexableAttributeContent(&$asset, $changed_vars)
	{
		$plugin        =& $this->_getDBPlugin();
		$weightings    =  $this->getWeightings($asset->id);
		$index_content =  Array();
		$type_code     =  $asset->type();
		$reindex_all   =  in_array('all', $changed_vars);

		foreach ($asset->vars as $var_name => $var_data) {

			// only reindex the attributes that have been changed
			if (!in_array($var_name, $changed_vars) && !$reindex_all) {
				continue;
			}

			// if there is no weightings for this attribute, then its not indexable
			if (!isset($weightings[$var_name])) continue;

			// don't index attribute that are names or short names as we index these
			// as part of the asset components
			if ($var_name == 'name' || $var_name == 'short_name') {
				// continue;
			}

			// don't index passwords or serialise attributes
			if ($var_data['type'] == 'password' || $var_data['type'] == 'serialise') {
				continue;
			}

			$attribute =& $asset->getAttribute($var_name);
			$value = strtolower($attribute->getContent());
			if (empty($value)) continue;

			$comp_name = 'attr:'.$var_name;

			// if the type is text and its just a number representation
			// index the number as a whole (remove spaces so that the number does
			// not get indexed as two separate entries)

			if ($var_data['type'] == 'text') {
				$value = $this->_getRawNumberInText($value);
			}

			if ($var_data['type'] == 'datetime') {
				// At least there's something we can agree on - datetimes should
				// never be split apart
				$index_content[] = Array(
									'value'		=> $value,
									'type_code'	=> $type_code,
									'type'		=> $var_data['type'],
									'component'	=> $comp_name,
									'score'		=> $weightings[$var_name],
								   );
			} else {
				$index_content = array_merge($index_content, $plugin->splitIndexableContent($value, $type_code, $var_data['type'], $comp_name, $weightings[$var_name]));
			}
		}//end foreach

		return $index_content;

	}//end getIndexableAttributeContent()


	/**
	* Returns the content for the specfied asset
	*
	* @param object	&$asset	the asset to obtain the indexable content for
	*
	* @return array
	* @access public
	*/
	function getIndexableContent(&$asset)
	{
		$plugin        =& $this->_getDBPlugin();
		$index_content =  Array();
		$weightings    =  $this->getWeightings($asset->id);
		$type_code     =  $asset->type();

		// only index this if there is a weighting set
		if (isset($weightings['__contents__'])) {

			// ask the asset for its content, and store it under a contents component
			// we don't want to index any tags so strip them

			$contents = strtolower($asset->getContent());
			// strip tags and replace with whitespace so as to preserve word boundaries
			$contents = preg_replace('/<\/?[^>]+>/i', ' ', $contents);
			$contents = html_entity_decode($contents, ENT_NOQUOTES);

			// html_entity_decode does not converts '$nbsp;' into an empty string.
			// Check the function documentation from http://www.php.net
			$contents = str_replace('&nbsp;', ' ', $contents);

			if (trim($contents) != '') {
				$index_content = $plugin->splitIndexableContent($contents, $type_code, 'text', '__contents__', $weightings['__contents__']);
			}
		}

		return $index_content;

	}//end getIndexableContent()


	/**
	* Adds the sepecifed index content for the specified assetid to the indexing system
	*
	* @param string	$assetid		the assetid to index the content under
	* @param array	&$index_content	the content to index
	*
	* @return boolean
	* @access public
	*/
	function addIndexableContent($assetid, &$index_content)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return FALSE;
		if (!is_array($index_content)) return FALSE;

		$param_data = Array();
		$keys = array_keys($index_content);
		foreach ($keys as $key) {
			$data =& $index_content[$key];
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

					$value = $this->_getRawNumberInText($value);
					// filter noise words
					if (!$this->isWordIndexable($value)) continue 2; // continue the foreach loop

				// NOTE: break is omitted on purpose
				case 'thesaurus':
				case 'select':
					$value = strtolower($value);
				break;

				default:
					// nothing yet
			}

			$param_data[] = Array(
								'value'		=> $value,
								'type_code'	=> $data['type_code'],
								'type'		=> $data['type'],
								'component'	=> $data['component'],
								'score'		=> $data['score'],
							);

		}//end foreach

		if (empty($param_data)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$insert_st = $db->prepare(
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
		assert_valid_db_result($insert_st);
		$result = $db->executeMultiple($insert_st, $param_data);
		assert_valid_db_result($result);

		$db->freePrepared($insert_st);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end addIndexableContent()


	/**
	* Flush the keydata for an asset
	*
	* @param int	$assetid	ID of the asset
	* @param array	$components	the components to which the keydata pertains (e.g. metadata or attribute)
	*
	* @return void
	* @access public
	*/
	function flushIndexableContent($assetid, $components=Array())
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return;
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'DELETE FROM
					sq_sch_idx
				WHERE
					assetid = '.$db->quoteSmart((string) $assetid);
		if (!empty($components)) {
			for (reset($components); NULL !== ($key = key($components)); next($components)) {
				$components[$key] = $db->quoteSmart((string) $components[$key]);
			}
			$sql .= ' AND component IN ('.implode(',', $components).')';
		}

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end flushIndexableContent()


	/**
	* Returns a list of components that the specified asset has indexed
	*
	* @param string	$assetid	the assetid of the wanted components
	*
	* @return array
	* @access public
	*/
	function getIndexedComponents($assetid)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT
					component
				FROM
					sq_sch_idx
				WHERE
					assetid = '.$db->quoteSmart((string) $assetid);

		$components = $db->getCol($sql);
		assert_valid_db_result($components);

		return $components;

	}//end getIndexedComponents()


	/**
	* Get ids of assets having same words as the supplied asset
	*
	* @param string	$source_id	ID of the asset
	* @param string	$type		type of the asset to restrict the search by
	*
	* @return array
	* @access public
	*/
	function getAssetidsByWordIntersection($source_id, $type=NULL)
	{
		$plugin =& $this->_getDBPlugin();
		$result = $plugin->getAssetidsByWordIntersection($source_id, $type);

		return $result;

	}//end getAssetidsByWordIntersection()


	/**
	* Returns a list of indexable compents for all assets
	*
	* @return array
	* @access public
	*/
	function getIndexableAssetComponents()
	{
		$components = Array(
						'__assetid__'			=> 'Asset ID',
						'__name__'				=> 'Asset Name',
						'__short_name__'		=> 'Asset Short Name',
						'__created__'			=> 'Created Date',
						'__updated__'			=> 'Updated Date',
						'__published__'			=> 'Published Date',
						'__status_changed__'	=> 'Status Changed Date',
						'__contents__'			=> 'Asset Contents',
					  );

		return $components;

	}//end getIndexableAssetComponents()


	/**
	* Returns a list of indexable components for a specified asset type
	*
	* @param string	$type_code	the type code
	*
	* @return array
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

		if (empty($weights)) {
			$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), TRUE, 'type_code');
			if (!isset($info[$assetid])) return FALSE;

			if (!$this->isTypeIndexable(array_get_index($info, $assetid, 0))) {
				return FALSE;
			}

			if (empty($weights)) {
				$weights = $this->getAssetTreeWeightings($assetid, FALSE);
			}
		}

		// if there is no indexed value in the array then we can assume
		// that indexing is on for this asset
		return (!isset($weights['indexed']) || $weights['indexed']);

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
		return (!isset($weights['indexed']) || $weights['indexed']);

	}//end isTypeIndexable()


	/**
	* Check if the word is indexable
	*
	* Check is performed according to the current indexing parameters
	*
	* @param string	$word	word to check
	*
	* @return boolean
	* @access public
	*/
	function isWordIndexable($word)
	{
		$plugin =& $this->_getDBPlugin();

		// Make sure db constraints aren't breached
		$max_length = $plugin->getMaxWordLength();
		if (($max_length > 0) && (strlen($word) > $max_length)) {
			return FALSE;
		}

		$min_length = $this->attr('min_word_length');
		return $this->isWhiteWord($word) || (strlen($word) >= $min_length && !$this->isNoiseWord($word));

	}//end isWordIndexable()


	/**
	* Checks to see if the specified text is just a number representation.
	*
	* If the text is just a number representation, the number is returned with its
	* spaces and delimiters stripped, otherwise the string is returned unchanged
	*
	* @param string	$value	the text to check
	*
	* @return string
	* @access private
	*/
	function _getRawNumberInText($value)
	{
		if (is_array($value)) return $value;
		// check to see if this value is a raw number
		if (!preg_match('/[a-zA-Z]+/', $value)) {
			$pattern = Array(',', '+');
			$new_value = str_replace($pattern, '', $value);
			$new_value = preg_replace('/\s+/', '', $new_value);

			// if after removal of whitespace and , and + characters
			// this is just a raw number, index it without spaces
			if (preg_match('/^\d+$/', $new_value)) {
				$value = $new_value;
			}
		}
		return $value;

	}//end _getRawNumberInText()


//--        METADATA INDEXING        --//


	/**
	* Event fired when metadata manager wants some metadata reindexed
	*
	* @param object	&$broadcaster	the asset that triggered the event (should be meta. mgr)
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Asset ID is set as $vars['assetid']
	*								Schema Asset IDs is set as $vars['schemaids']
	*
	* @return void
	* @access public
	*/
	function onMetadataUpdate(&$broadcaster, $vars=Array())
	{
		$this->reindexMetadata($broadcaster->id, $vars);

	}//end onMetadataUpdate()


	/**
	* Event fired when metadata manager wants some metadata deleted
	*
	* @param object	&$broadcaster	the asset that triggered the event (should be meta. mgr)
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Asset ID is set as $vars['assetid']
	*								Schema Asset IDs is set as $vars['schemaids']
	*
	* @return void
	* @access public
	*/
	function onMetadataDeleted(&$broadcaster, $vars=Array())
	{
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$fieldids = array_keys($mm->getMetadataFields(Array($vars['schemaid'])));
		for (reset($fieldids); NULL !== ($key = key($fieldids)); next($fieldids)) {
			$fieldids[$key] = 'metadata:'.$fieldids[$key];
		}

		$this->flushIndexableContent($broadcaster->id, $fieldids);

	}//end onMetadataDeleted()


	/**
	* Re-indexes the metadata components for the specified asset
	*
	* @param string	$assetid	the assetid to reindex
	* @param array	$vars		the schemas to renidex
	*
	* @return void
	* @access public
	*/
	function reindexMetadata($assetid, $vars=Array())
	{
		assert_valid_assetid($assetid);
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		if (in_array('all', $vars)) {
			$fieldids = $mm->getMetadataFields($mm->getSchemas($assetid));
		} else if (in_array('schemaid', array_keys($vars))) {
			$fieldids = $mm->getMetadataFields($vars['schemaid'], TRUE);
		} else {
			$fieldids = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($vars['fieldids'], Array(), TRUE, 'type_code');
		}

		if (empty($fieldids)) {
			$fieldids = $mm->getMetadataFields($mm->getSchemas($assetid, TRUE));
		}

		$flush_fieldids = Array();
		foreach (array_keys($fieldids) as $fieldid) {
			$flush_fieldids[] = 'metadata:'.$fieldid;
		}

		if (!empty($flush_fieldids)) {
			$this->flushIndexableContent($assetid, $flush_fieldids);
		}

		if (!$this->isAssetIndexable($assetid)) return;

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		$this->addIndexableContent($assetid, $this->getIndexableMetadataContent($asset, $fieldids));

	}//end reindexMetadata()


	/**
	* Returns the indexable metadata content for the specified asset
	*
	* @param object	&$asset		The asset to get the indexable content fors
	* @param array	$fieldids	Array of fieldid => type_code to limit to
	*
	* @return array
	* @access public
	*/
	function getIndexableMetadataContent(&$asset, $fieldids=Array())
	{
		$index_content = Array();
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$plugin =& $this->_getDBPlugin();
		$type_code = $asset->type();

		if (is_null($asset)) return $index_content;

		if (empty($fieldids)) {
			$fieldids = $mm->getMetadataFields($mm->getSchemas($asset->id, TRUE));
		}

		$metadata = $mm->getMetadata($asset->id);

		foreach ($fieldids as $fieldid => $field_type) {

			// dont index this fieldid if getMetadataWeighting return false
			if (FALSE === ($weighting = $this->getMetadataWeighting($asset->id, $fieldid))) {
				continue;
			}

			$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid, $field_type);

			if (isset($metadata[$field->id]['value'])) {
				$value = $metadata[$field->id]['value'];
			} else {
				$value = $field->attr('default');
			}
			$value_components = Array();
			Metadata_Field::decodeValueString($value, $value, $value_components);

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

				$keywords = retrieve_keywords_replacements($value, '.');
				replace_keywords($value, $mm->generateKeywordReplacements($asset, $keywords, FALSE));

				switch ($field_type) {
					case 'metadata_field_select':
						if ($field->attr('multiple')) {
							$plugin =& $this->_getDBPlugin();
							$value = $plugin->handleMultipleMetadataSelect($value);
						}

						$key_type = 'selection';
					break;

					case 'metadata_field_thesaurus':
						$key_type = 'thesaurus';
					break;

					default:
						$key_type = 'text';
						$value = $this->_getRawNumberInText($value);
				}

				$index_content = array_merge($index_content, $plugin->splitIndexableContent($value, $type_code, $key_type, 'metadata:'.$field->id, $weighting));
			}//end else

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($field);
			unset($field);

		}//end foreach fieldids

		return $index_content;

	}//end getIndexableMetadataContent()


//--        WEIGHTINGS        --//


	/**
	* Returns the weightings for an assets core components and its attributes
	*
	* The Following presedence rules are followed in order when finding the weightings for
	* a particular asset:
	* <ul>
	*  <li> Any explicit weightings for the asset id are checked </li>
	*  <li> Any weightings for a tree in which the asset resides are checked</li>
	*  <li> Any weightings for a asset type are checked </li>
	*  <li> Any global weightings for all asset types are checked </li>
	*  <li> All components are returned a default weighing of 1</li>
	* </ul>
	*
	* @param string	$assetid	the assetid to obtain the weightings for
	*
	* @return array
	* @access public
	*/
	function getWeightings($assetid)
	{
		// firstly, check if there are any weightings specific to this asset
		$weightings = $this->getAssetWeightings($assetid);

		if (empty($weightings)) {

			// if we can't find any weightings specific to this asset, check
			// to see if there are any tree weightings for this asset
			$weightings = $this->getAssetTreeWeightings($assetid, FALSE);

			// if we can't find any tree weightings that this asset exists under
			// check the asset type weightings.
			if (empty($weightings)) {
				$type_code = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), TRUE, 'type_code');
				$type_code = array_get_index($type_code, $assetid, '');
				if (!empty($type_code)) {
					$weightings = $this->getAssetTypeWeightings($type_code);
				}

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
	* <ul>
	*  <li> Any explicit weightings for the asset id are checked </li>
	*  <li> Any weightings for a tree in which the asset resides are checked </li>
	*  <li> Any weightings for a asset type are checked </li>
	*  <li> Any global weightings for all asset types are checked </li>
	*  <li> A default weighing of 1 is returned </li>
	* </ul>
	*
	* @param string	$assetid	the assetid to get the weightings for
	* @param string	$fieldid	the fieldid of the metadata field
	*
	* @return mixed array|boolean
	* @access public
	*/
	function getMetadataWeighting($assetid, $fieldid)
	{
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$section_link = $GLOBALS['SQ_SYSTEM']->am->getLink($fieldid, SQ_LINK_TYPE_2, 'metadata_section', TRUE, NULL, 'minor');
		if (empty($section_link)) {
			// the field is not attached to a section, so return default weighting
			return 1;
		}

		$schema_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($section_link['majorid'], SQ_LINK_TYPE_2, 'metadata_schema', TRUE, 'minor');
		if (empty($schema_links)) {
			// the section is not attached to a schema, so return default weighting
			return 1;
		}

		$sectionid    = $section_link['majorid'];
		$schemaids    = Array();

		foreach ($schema_links as $schema_info) {
			$schemaids[] = $schema_info['majorid'];
		}

		$info      = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($fieldid), Array(), TRUE, 'type_code');
		$type_code = $info[$fieldid];

		$weight_sources = Array(
							'Asset'		=> $assetid,
							'AssetTree'	=> $assetid,
							'AssetType'	=> $type_code,
							// also check the global metadata weightings
							'AssetTree'	=> $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder'),
						  );

		foreach ($weight_sources as $type => $key) {
			$fn = 'get'.$type.'weightings';
			$weightings = $this->$fn($key);
			if (!empty($weightings) && isset($weightings['metadata_weights'])) {
				$weight = $this->_getLocalMetadataWeightings($weightings['metadata_weights'], $fieldid, $sectionid, $schemaids);
				// if the weight is false then we don't want to index this field
				if ($weight === FALSE) return FALSE;
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
	* Returns the weighting for the field or false if the fieldid is not to be indexed
	* -1 if no weighting could be found
	*
	* @param array	$weightings	the weightigns to search
	* @param string	$fieldid	the fieldid of the metadata field
	* @param string	$sectionid	the sectionid that the field is a child of
	* @param string	$schemaids	the schemaid that the section is a child of
	*
	* @return int
	* @access private
	*/
	function _getLocalMetadataWeightings($weightings, $fieldid, $sectionid, $schemaids)
	{
		if (!empty($weightings)) {
			// check to see if there is an explicit weighting for the field
			if (isset($weightings[$fieldid])) {
				if (!$weightings[$fieldid]['indexed']) return FALSE;
				return $weightings[$fieldid]['weight'];
			} else {
				// check for a weighting for the section that the field is under
				if (isset($weightings[$sectionid])) {
					if (!$weightings[$sectionid]['indexed']) return FALSE;
					return $weightings[$sectionid]['weight'];
				} else {
					// check all the schemas that the section belongs under
					// its possible that this field is in multiple schemas, so find the
					// hightest weighting from all the schemas that it is under
					$highest_weight = -1;

					// check the index flag on all the schemas, and only turn off indexing
					// for this fieldid if all schemas have indexing off
					$index = FALSE;

					foreach ($schemaids as $schemaid) {
						if (isset($weightings[$schemaid])) {
							if ($weightings[$schemaid]['indexed']) $index = TRUE;
							if ($weightings[$schemaid]['weight'] > $highest_weight) {
								$highest_weight = $weightings[$schemaid]['weight'];
							}
						}
					}
					if (!$index && $highest_weight !== -1) {
						 return FALSE;
					 }
					if ($highest_weight !== -1) return $highest_weight;
				}
			}
		}//end if !empty($weightings
		// no weight could be found
		return -1;

	}//end _getLocalMetadataWeightings()


	/**
	* Returns the Asset Type weightings for the specified asset type, or all
	* asset types if the specified type code is null
	*
	* @param string		$type_code			the type code of the wanted weightings, or null
	*										for all asset type weightings
	* @param boolean	$strict_type_code	strict or inherit type code
	*
	* @return array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getAssetTypeWeightings($type_code=NULL, $strict_type_code=FALSE)
	{
		$root_folder_assetid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder');
		// type weightings are stored under the root folder tree weightings
		$weights = $this->getAssetTreeWeightings($root_folder_assetid, TRUE, TRUE);

		$type_weights = array_get_index($weights, 'type_weights', Array());

		if (is_null($type_code)) return $type_weights;

		if (!$strict_type_code) {
			$type_hier = Array($type_code);
			$type_hier += $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, TRUE);
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
	* @param boolean	$strict_assetid		if TRUE only weightings that have been
	*										customised explicitly using the specified assetid
	*										will be returned, otherwise the closest weighting
	*										customisation will be returned
	* @param boolean	$strict_type_code	if TRUE only weightings matching the asset type of the
	*										specified assetid will be returned
	*
	* @return array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getAssetTreeWeightings($assetid=NULL, $strict_assetid=TRUE, $strict_type_code=FALSE)
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
					$type_code = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), TRUE, 'type_code');
					$type_code = array_get_index($type_code, $assetid, '');

					// firstly, check to see if there is an explicit entry for this assetid
					if (isset($asset_tree_weightings[$assetid]['type_weights'][$type_code])) {
						return $asset_tree_weightings[$assetid]['type_weights'][$type_code];
					}

					// check to see if there are customisations for all asset types
					if (isset($asset_tree_weightings[$assetid]['type_weights']['asset'])) {
						return $asset_tree_weightings[$assetid]['type_weights']['asset'];
					}

					// if type code not found for some unknown reason, default to base asset
					if (empty($type_code)) $type_code = 'asset';

					// there is no explicit weighting customisation for the assetid
					// so do a tree lookup for the closest one. getParents ORDER's by treeid
					// by default so we can be garunteed that the closest assetids are at the top of the array
					$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($assetid);
					$closest_parentid = 0;

					// find the parent asset types of this type code
					if (!$strict_type_code) {
						$type_hier = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, TRUE);
						// add the current type code so that we can also find
						// weightings for this type, not just its parent types
						array_unshift($type_hier, $type_code);
					} else {
						$type_hier = Array($type_code);
					}

					// Search by type code first; that way we will always find the
					// closest asset type to what we are using
					foreach ($type_hier as $type_code) {
						foreach (array_keys($parents) as $parentid) {
							if (in_array($parentid, array_keys($asset_tree_weightings))) {
								if (isset($asset_tree_weightings[$parentid]['type_weights'])) {
									if (in_array($type_code, array_keys($asset_tree_weightings[$parentid]['type_weights']))) {
										$closest_parentid = $parentid;
										break;
									}
								}
							}
						}

						// If we already have a parent, break out
						if ($closest_parentid != 0) break;
					}

					if ($closest_parentid != 0) {
						return $asset_tree_weightings[$closest_parentid]['type_weights'][$type_code];
					}
				}//end else strict_assetid
			} else {
				return $asset_tree_weightings;
			}//end if !is_null(assetid)
		}//end if file_exists

		return Array();

	}//end getAssetTreeWeightings()


	/**
	* Saves the asset tree weightings to file
	*
	* @param array	$asset_tree_weightings	the complete asset tree weightings to save
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
	* @return array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getAssetWeightings($assetid=NULL)
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
	* @param array	$asset_weightings	the array of weights to save
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
	* @return array
	* @access public
	* @see saveTypeWeightings()
	*/
	function getMetadataWeightings($assetid=NULL)
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
	* @param array	$metadata_weightings	the complete metadata weightings to save
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
	* @param array	$weightings	the weightings to save
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
			return FALSE;
		}
		$weightings_file = $this->data_path.'/'.$type.'_weightings.inc';

		$output = '<'.'?php'."\n".' $'.$type.'_weightings = ';
		$output .= var_export($weightings, TRUE);
		$output .= "\n?".'>';

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		create_directory($this->data_path);
		if (!string_to_file($output, $weightings_file)) {
			trigger_error('Could not write out the '.str_replace('_', ' ', $type).' weightings to file', E_USER_WARNING);
			return FALSE;
		}
		return TRUE;

	}//end _saveWeightings()


//--        KEYWORD EXTRACTION        --//


	/**
	* Extracts keywords from the indexing table and ranks them in order of descending importance
	*
	* @param object		&$asset				The asset whose keywords are to be extracted
	* @param boolean	$include_metadata	If false, metadata components are excluded
	* @param boolean	$include_scores		Determines whether scores are returned (true) or just
	*										the keywords (false). Affects return value.
	*
	* @return array
	* @access public
	*/
	function extractKeywords(&$asset, $include_metadata=FALSE, $include_scores=FALSE)
	{
		$plugin =& $this->_getDBPlugin();
		return $plugin->extractKeywords($asset, $include_metadata, $include_scores);

	}//end extractKeywords()


//--        SEARCH PROCESSING        --//


	/**
	* Generates a word list from the search string
	*
	* @param string	$words	the words string
	*
	* @return array
	* @access public
	*/
	function generateWordList($words)
	{
		$plugin =& $this->_getDBPlugin();
		return $plugin->generateWordList($words);

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
	* <pre>
	* 'requester'	=> ID of asset that requested the search
	* 'asset_types'	=> Array of type code => whether to inherit
	* 'statuses'	=> Array of asset statuses, or empty array for default of live+
	* 'roots'		=> Array of assetids to use as roots for the search
	* 'root_logic'	=> 'AND'|'OR', depending on whether we want a result to be under all roots or any root
	* 'fields'		=> Array(
	*					field_name	=> Array(
	*									'words'				=> 'cat sat mat',
	*									'word_logic'		=> 'AND'|'OR' (all words or any word, default 'AND')
	*									'data_source_logic'	=> 'AND'|'OR' (all DSes or any DS, default 'OR')
	*									'data_sources'		=> Array(
	*															i	=> Array(
	*																	'type'		=> 'asset_attrib' for eg
	*																	'params'	=> Array(...)
	*																   ),
	*														   ),
	*								   ),
	*				   ),
	* 'field_logic'	=> 'AND'|'OR', depending on whether we want results that match all fields or any field
	* 'exclude'		=> Array of words to exclude from results
	* </pre>
	* @param boolean	$include_context	Whether to include the context in which the match was found
	*
	* @return array
	* @access public
	*/
	function processSearch($search_info, $include_context=FALSE)
	{

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc'; // will definitely need this later.

		  /////////////////////////////
		 //  CONSTRUCT CACHING KEY  //
		/////////////////////////////
		// we need the cache manager to cache the results of queries
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		if (empty($search_info['roots'])) {
			$search_cache_key = 'systemWide';
		} else {
			$search_cache_key = implode(',', $search_info['roots']);
		}
		if (isset($search_info['statuses'])) {
			$search_cache_key .= implode(':', $search_info['statuses']);
		}
		if (isset($search_info['asset_types'])) {
			foreach ($search_info['asset_types'] as $type => $inherit) {
				$search_cache_key .= $type.($inherit ? 'i' : 'n').':';
			}
		}
		$search_cache_key .= 'RL='.array_get_index($search_info, 'root_logic', 'OR');
		if ($include_context) {
			$search_cache_key .= 'includeContext';
		}
		$this->_tmp['cache_key'] = $search_cache_key;
		$this->_tmp['cm'] =& $cm;

		  //////////////////////
		 //  PREPARE FIELDS  //
		//////////////////////
		foreach ($search_info['fields'] as $field_name => $field_details) {
			$this->_prepareSearchField($search_info['fields'][$field_name]);
		}

		  ////////////////////////////////////////////////////
		 //  HANDLE ANY SHADOW ASSET OR BRIDGE ROOT NODES  //
		////////////////////////////////////////////////////
		if (!empty($search_info['roots'])) {
			$root_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($search_info['roots']);
			$shadow_results = Array();
			foreach ($search_info['roots'] as $i => $assetid) {
				$root = $root_info[$assetid];
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($root['type_code']);
				// assert class implements bridge and contains method
				if (implements_interface($root['type_code'], 'bridge') && is_callable(Array($root['type_code'], 'processSearch'))) {
					$shadow_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					$shadow_results = $this->_combineResults($shadow_results, $shadow_asset->processSearch($search_info, $include_context), array_get_index($search_info, 'root_logic', 'OR'));
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($shadow_asset);
					unset($search_info['roots'][$i]);
					$shadow_processed = TRUE;
				}
			}
			if (empty($search_info['roots'])) {
				return $shadow_results;
			}
		}

		  ///////////////////////////////////
		 //  CONSTRUCT BASE SEARCH QUERY  //
		///////////////////////////////////
		$base_query = $this->constructBaseSearchQuery($search_info);
		$base_query['select'][] = 'a.assetid';
		$base_query['select'][] = 'SUM(ai.score) as search_score';
		$base_query['join'][] = 'INNER JOIN sq_sch_idx ai ON ai.assetid = a.assetid';

		if (!in_array('a.assetid', array_get_index($base_query, 'group_by', Array()))) {
			$base_query['group_by'][] = 'a.assetid';
		}
		if ($include_context) {
			$base_query['select'][] = 'ai.component as source';
			$base_query['group_by'][] = 'ai.component';
		}

		  //////////////////////////
		 //  PROCESS EACH FIELD  //
		//////////////////////////
		$field_logic = array_get_index($search_info, 'field_logic', 'OR');
		$final_results = NULL;
		$term_totals = Array();

		foreach ($search_info['fields'] as $field_name => $field_details) {
			$field_results = NULL;
			$word_logic = array_get_index($field_details, 'word_logic', 'AND');
			$data_source_logic = array_get_index($field_details, 'data_source_logic', 'OR');

			if (empty($field_details['words'])) continue;

			if (isset($field_details['words']['from']) && isset($field_details['words']['to'])) {
				$search_type = 'date';
			} else if (array_key_exists('lower', $field_details['words']) && array_key_exists('upper', $field_details['words'])) {
				// need to use array_key_exists()
				$search_type = 'numeric';
			} else {
				$search_type = 'word';
			}

			$all_source_results = Array();
			$all_source_assetids = Array();

			foreach ($field_details['data_sources'] as $data_source) {
				if ($search_type == 'numeric') {
					$data_source_results = $this->_processNumericSearch($field_details['words'], $data_source, $base_query);
				} else if ($search_type == 'date') {
					$data_source_results = $this->_processDateSearch($field_details['words'], $data_source, $base_query);
				} else {
					$data_source_results = NULL;

					foreach ($field_details['words'] as $search_term) {
						$term_results = $this->_processWordSearch($search_term, $data_source, $base_query, $word_logic);

						if (!isset($all_source_results[$search_term])) {
							$all_source_results[$search_term] = Array();
						}
						$all_source_results[$search_term] = $all_source_results[$search_term] + $term_results;

						// Records all the assets in which we find the term.
						$all_source_assetids = $all_source_assetids + $term_results;

						$term_totals[$search_term] = array_get_index($term_totals, $search_term, 0) + count($term_results);
						$data_source_results = $this->_combineResults($data_source_results, $term_results, $word_logic);
					}
				}
				$field_results = $this->_combineResults($field_results, $data_source_results, $data_source_logic);
			}

			$cross_source_results = $all_source_assetids;
			foreach ($all_source_results as $term => $results) {
				$cross_source_results = array_intersect_assoc($cross_source_results, $results);
			}
			$field_results = $field_results + $cross_source_results;

			$final_results = $this->_combineResults($final_results, $field_results, $field_logic);
		}//end foreach $search_info['fields']

		  //////////////////////////////
		 //  PROCESS EXCLUDED TERMS  //
		//////////////////////////////
		if (!empty($search_info['exclude'])) {
			$plugin =& $this->_getDBPlugin();
			$result = $plugin->processExcludeQuery($this, $search_info['exclude'], $base_query);

			foreach ($result as $exclude_assetid) {
				if (isset($final_results[$exclude_assetid])) {
					unset($final_results[$exclude_assetid]);
				}
			}
		}

		// join any results from bridges
		if (!empty($shadow_results)) {
			$final_results = $this->_combineResults($final_results, $shadow_results, array_get_index($search_info, 'root_logic', 'OR'));
		}

		  ///////////////////////
		 //  LOG THIS SEARCH  //
		///////////////////////
		if (!empty($search_info['requester'])) {
			$log_contents['terms']   = $term_totals;
			$log_contents['results'] = count($final_results);
			$log_contents['assetid'] = $search_info['requester'];
			log_write($log_contents, 'search');
		}

		return $final_results;

	}//end processSearch()


	/**
	* Prepares the supplied search field for searching
	*
	* If all datasources are of type datetime, makes sure the 'words' array is appropriately set up as a
	* date range.  Otherwise gets the words from the search string, makes them into an array and removes
	* unindexable words. Data sources that are not properly initialised are removed
	*
	* @param array	&$field_details	One entry from the 'fields' array in processSearch()'s search_info parameter
	*
	* @return void
	* @access private
	*/
	function _prepareSearchField(&$field_details)
	{
		if (empty($field_details['data_sources'])) {
			$field_details['data_sources'] = Array();
			return;
		}

		// figure out if we should treat this field as a date/time range
		$is_datetime = TRUE;
		$is_numeric = FALSE;
		foreach ($field_details['data_sources'] as $i => $data_source) {
			switch ($data_source['type']) {

				case 'asset_attrib':
					$asset_type = array_get_index($data_source['params'], 'asset_type');
					$attrid = array_get_index($data_source['params'], 'attrid');
					if (empty($asset_type) || empty($attrid)) {
						unset($field_details['data_sources'][$i]);
						continue;
					}
					$attribute =& $GLOBALS['SQ_SYSTEM']->am->getAttribute($attrid);
					if (is_null($attribute)) {
						unset($field_details['data_sources'][$i]);
						continue;
					}
					if (!is_a($attribute, 'asset_attribute_datetime')) {
						$is_datetime = FALSE;
					}
					if (is_a($attribute, 'asset_attribute_int') || is_a($attribute, 'asset_attribute_float')) {
						$is_numeric = TRUE;
					}
					if (!$is_datetime || $is_numeric) {
						break 2;
					}
				break;

				case 'metadata':
					if (empty($data_source['params']['assetid'])) {
						unset($field_details['data_sources'][$i]);
						continue;
					}
					$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['params']['assetid']);
					if (is_null($metadata_field)) {
						unset($field_details['data_sources'][$i]);
						continue;
					}
					if (!is_a($metadata_field, 'metadata_field_date')) {
						$is_datetime = FALSE;
						break(2);
					}
				break;

				case 'standard':
					$asset_field = array_get_index($data_source['params'], 'field');
					if (empty($asset_field)) {
						unset($field_details['data_sources'][$i]);
						continue;
					}
					if (isset($this->standard_text_fields[$asset_field])) {
						$is_datetime = FALSE;
						break(2);
					} else if (!isset($this->standard_date_fields[$asset_field])) {
						trigger_error('Invalid standard field "'.$asset_field.'"', E_USER_WARNING);
						break(2);
					}
				break;

				case 'include_all':
					$is_datetime = FALSE;
					break(2);
				break;

				default:
					trigger_error('Unknown data source type '.$data_source['type'], E_USER_WARNING);
				break;

			}//end switch
		}//end foreach data source

		if ($is_datetime) {
			$from = array_get_index($field_details['words'], 'from', '---------- --:--:--');
			$to = array_get_index($field_details['words'], 'to', '---------- --:--:--');
			if ($from != '---------- --:--:--') {
				$from = str_replace('--:--:--', '00:00:00', $from);
			}
			if ($to != '---------- --:--:--') {
				$to = str_replace('--:--:--', '23:59:59', $to);
			}
			$field_details['words'] = Array('from' => $from, 'to' => $to);
		} else if ($is_numeric) {
			$lower = array_get_index($field_details['words'], 'lower', NULL);
			$upper = array_get_index($field_details['words'], 'upper', NULL);
			$field_details['words'] = Array('lower' => $lower, 'upper' => $upper);
		} else {
			// remove unindexable words
			$words = Array();

			foreach ($this->generateWordList($field_details['words']) as $word) {
				if ($this->isWordIndexable($word)) {
					$words[] = strtolower($word);
				}
			}
			$field_details['words'] = $words;
		}

	}//end _prepareSearchField()


	/**
	* Processes a search for a particular word in a particular data source
	*
	* @param string	$search_term	Word to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	* @param string	$word_logic		Word logic (for DBs that don't split up their
	*								terms here, ie. Oracle)
	*
	* @return array
	* @access public
	*/
	function _processWordSearch($search_term, $data_source, $base_query, $word_logic='AND')
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$cache_key = $this->_tmp['cache_key'].$data_source['type'];
		switch ($data_source['type']) {
			case 'asset_attrib':
				$cache_key .= $data_source['params']['attrid'];
			break;

			case 'metadata':
				$cache_key .= $data_source['params']['assetid'];
			break;

			case 'standard':
				$cache_key .= $data_source['params']['field'];
			break;
		}
		$cache_key .= $search_term;

		$result = $this->_tmp['cm']->loadFromCache($this->id, $this->type(), $cache_key);
		if (empty($result)) {
			$plugin =& $this->_getDBPlugin();
			$result = $plugin->processWordSearch($this, $search_term, $data_source, $base_query, $word_logic);
			$r = $this->_tmp['cm']->saveToCache($this->id, $this->type(), $cache_key, serialize($result));
		} else {
			$result = unserialize($result);
		}
		return $result;

	}//end _processWordSearch()


	/**
	* Search for a date range in a particular data source
	*
	* @param string	$date_range		Range of dates to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	*
	* @return void
	* @access private
	*/
	function _processDateSearch($date_range, $data_source, $base_query)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$cache_key = $this->_tmp['cache_key'].$data_source['type'];
		switch ($data_source['type']) {
			case 'asset_attrib':
				$cache_key .= $data_source['params']['attrid'];
			break;

			case 'metadata':
				$cache_key .= $data_source['params']['assetid'];
			break;

			case 'standard':
				$cache_key .= $data_source['params']['field'];
			break;
		}
		$cache_key .= 'from'.$date_range['from'].'to'.$date_range['to'];

		$result = $this->_tmp['cm']->loadFromCache($this->id, $this->type(), $cache_key);
		if (empty($result)) {
			$plugin =& $this->_getDBPlugin();
			$result = $plugin->processDateSearch($this, $date_range, $data_source, $base_query);
			$r = $this->_tmp['cm']->saveToCache($this->id, $this->type(), $cache_key, serialize($result));
		} else {
			$result = unserialize($result);
		}
		return $result;

	}//end _processDateSearch()


	/**
	* Search for a numeric range in a particular data source
	*
	* @param string	$numeric_range	Range of numbers to search for
	* @param array	$data_source	Details of the data source to search in
	* @param array	$base_query		The base of the sql query to use
	*
	* @return void
	* @access private
	*/
	function _processNumericSearch($numeric_range, $data_source, $base_query)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$cache_key = $this->_tmp['cache_key'].$data_source['type'];
		switch ($data_source['type']) {
			case 'asset_attrib':
				$cache_key .= $data_source['params']['attrid'];
			break;

			case 'metadata':
				$cache_key .= $data_source['params']['assetid'];
			break;

			case 'standard':
				$cache_key .= $data_source['params']['field'];
			break;
		}
		$cache_key .= 'l'.$numeric_range['lower'].'u'.$numeric_range['upper'];

		$result = $this->_tmp['cm']->loadFromCache($this->id, $this->type(), $cache_key);
		if (empty($result)) {
			$plugin =& $this->_getDBPlugin();
			$result = $plugin->processDateSearch($this, $numeric_range, $data_source, $base_query);
			$r = $this->_tmp['cm']->saveToCache($this->id, $this->type(), $cache_key, serialize($result));
		} else {
			$result = unserialize($result);
		}
		return $result;

	}//end _processNumericSearch()


	/**
	* Combine new results with results so far
	*
	* If $results_so_far is null, $new_results are always returned.  Otherwise if logic is 'AND'
	* the return value contains only the results that are in both the first two params, with
	* search scores added.  If logic is 'OR' then the return value contains all results that are
	* in either of the first two params, with search scores added for common results. This function
	* can handle results with or without context (ie getAll or getAssoc return format)
	*
	* @param array	$results_so_far	Results so far.  Null if no results so far
	* @param array	$new_results	New results
	* @param string	$logic			'AND'|'OR'
	*
	* @return array
	* @access public
	*/
	function _combineResults($results_so_far, $new_results, $logic)
	{
		if (is_null($results_so_far)) {
			return $new_results;
		} else {
			if ($logic == 'OR') {
				$res_results = $results_so_far;
			} else {
				$res_results = Array();
			}
			if (isset($results_so_far[0])) {
				// getAll format, rather than getAssoc format
				foreach ($new_results as $new_result) {
					$added = FALSE;
					$found = FALSE;
					foreach ($results_so_far as $i => $old_result) {
						if ($old_result['assetid'] == $new_result['assetid']) {
							$found = TRUE;
							if ($old_result['source'] == $new_result['source']) {
								if ($logic == 'AND') {
									$old_result['search_score'] += $new_result['search_score'];
									$res_results[] = $old_result;
								} else {
									$res_results[$i]['search_score'] += $new_result['search_score'];
								}
								$added = TRUE;
								break;
							} else if ($logic == 'AND') {
								// the assetid is common even though the source isn't, so the
								// existing entry deserves to stay
								$res_results[] = $old_result;
							}
						}
					}
					if (!$added && ($found || ($logic == 'OR'))) {
						// we haven't added it yet and either the logic is "OR" or the assetid is common
						// so we add the new result now
						$res_results[] = $new_result;
					}
				}
			} else {
				foreach ($new_results as $assetid => $score) {
					if ($logic == 'OR') {
						$res_results[$assetid] = $score + array_get_index($results_so_far, $assetid);
					} else if (isset($results_so_far[$assetid])) {
						$res_results[$assetid] = $score + $results_so_far[$assetid];
					}
				}
			}
			return $res_results;
		}//end else

	}//end _combineResults()


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
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc'; // will definitely need this later.
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$query_comps = $this->constructBaseSearchQuery($search_info);
		if (empty($query_comps)) return Array();
		$query_comps['select'][] = 'distinct a.assetid';
		$query_comps['select'][] = '1 as weight';
		$query_comps['join'][] = 'INNER JOIN sq_sch_idx s ON a.assetid = s.assetid';

		$search_sql = implode_sql($query_comps);
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

		// get results from any shadow asset root nodes
		$root_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($search_info['roots']);

		foreach ($root_info as $assetid => $root) {
			$shadow_results = Array();
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($root['type_code']);

			// assert class implements bridge and contains method
			if (implements_interface($root['type_code'], 'bridge') && is_callable(Array($root['type_code'], 'processBasicSearch'))) {
				$shadow_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$shadow_results += $shadow_asset->processBasicSearch($search_info);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($shadow_asset);
			}
		}

		if (!empty($shadow_results)) {
			$search_results += $shadow_results;
		}

		return $search_results;

	}//end processBasicSearch()


	/**
	* Spell-checks a word and returns a list of suggestions if available
	*
	* This function checks for the existence of the pSpell extension. If it does
	* not exist (eg. Windows server), or an error otherwise occurs in the loading
	* of the pSpell instance, we will proceed as if no suggestions are available.
	*
	* @param string	$word		the word to be checked
	* @param string	$language	the language to be used to spell-check
	*
	* @return array
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

		$check_spelling = TRUE;

		// ignore numbers
		if ($check_spelling) {
			$converted_word = (int)$word;
			if ((string)$converted_word == $word) {
				$check_spelling = FALSE;
			}
		}

		// ignore words in all uppercase
		if ($check_spelling) {
			$converted_word = strtoupper($word);
			if ($converted_word == $word) $check_spelling = FALSE;
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
	* Populates a search query array given an array of search settings
	*
	* Accepts the same search info array as processSearch()
	* Does not put the search index table or anything like that in the query
	* Returns components of search query in a format ready for implode_sql()
	*
	* @param array	$search_info	an array of search settings
	*
	* @return array
	* @access public
	* @see processSearch(), general_occasional::implode_sql
	*/
	function constructBaseSearchQuery($search_info)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$query = Array(
					'select'		=> Array(),
					'from'			=> Array('sq_ast a'),
					'where'			=> Array(),
					'where_joiner'	=> 'AND',
					'order_by'		=> Array(),
				 );

		// TREE LOCATIONS
		if (!empty($search_info['roots'])) {
			$root_logic = array_get_index($search_info, 'root_logic', 'OR');
			// get the treeids of our search roots
			$treeid_sql = 'SELECT l.minorid, t.treeid
						   FROM sq_ast_lnk_tree t
							   JOIN sq_ast_lnk l on t.linkid = l.linkid
						   WHERE l.minorid IN ('.implode(',', $search_info['roots']).')';
			// getAssoc only gives us the first treeid for each minorid, which is actually just what we want!
			$root_tree_ids = $db->getAssoc($treeid_sql);
			assert_valid_db_result($root_tree_ids);

			if ($root_logic == 'AND') {
				foreach (array_values($search_info['roots']) as $i => $rootid) {
					$query['join'][] = 'INNER JOIN sq_ast_lnk l'.$i.' ON l'.$i.'.minorid = a.assetid'; // join the link tree and link tables
					$query['join'][] = 'INNER JOIN sq_ast_lnk_tree t'.$i.' ON l'.$i.'.linkid =  t'.$i.'.linkid'; // join them to the asset table

					$treeid = $root_tree_ids[$rootid];
					$query['where'][] = 't'.$i.'.treeid LIKE '.$db->quote($treeid.'%');
				}
			} else {
				$query['join'][] = 'INNER JOIN sq_ast_lnk l ON l.minorid = a.assetid';
				$query['join'][] = 'INNER JOIN sq_ast_lnk_tree t ON t.linkid = l.linkid';

				$treeid_wheres = Array();
				foreach ($root_tree_ids as $treeid) {
					$treeid_wheres[] = '(t.treeid LIKE '.$db->quote($treeid.'%').')';
				}
				if (!empty($treeid_wheres)) {
					$query['where'][] = implode(' OR ', $treeid_wheres);
				}
			}
		}//end if !empty($search_info['roots'])

		// ACCESS RESTRICTIONS
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if ($user_restrictions) {
			$query['join'][] = 'INNER JOIN sq_ast_perm p ON p.assetid = a.assetid';
			$query['join'][] = 'LEFT JOIN sq_vw_ast_role r ON p.userid = r.roleid';

			$public_userid = (String)$GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

			// sanity check; if the global user isn't set, assume we're a public user
			if (empty($GLOBALS['SQ_SYSTEM']->user->id) || $GLOBALS['SQ_SYSTEM']->user->id == $public_userid) {
				$userids = Array($public_userid);
			} else {
				$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				array_push($userids, $public_userid, $GLOBALS['SQ_SYSTEM']->user->id);
			}

			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$userids[$i] = $db->quoteSmart((String)$userids[$i]);
			}
			$usrids_str = implode(',', $userids);
			$query['where'][] = 'p.userid  IN ('.$usrids_str.') OR '.'r.userid  IN ('.$usrids_str.')';
			$query['where'][] = '(
						(p.permission = '.$db->quoteSmart((String)SQ_PERMISSION_READ).'
							AND (
									p.userid <> '.$db->quoteSmart($public_userid).'
									OR r.userid <> '.$db->quoteSmart($public_userid).'
									OR (p.userid = '.$db->quoteSmart($public_userid).' AND granted = 1)
									OR (r.userid = '.$db->quoteSmart($public_userid).' AND granted = 1)
								)
						)
						OR
						(p.permission > '.$db->quoteSmart((String)SQ_PERMISSION_READ).' AND p.granted = 1)
						)';
			$query['having'][] = 'MIN(p.granted) <> 0';
			$query['group_by'][] = 'a.assetid';
		}//end if

		// STATUS RESTRICTIONS
		if (!empty($search_info['statuses'])) {
			$statuses = $search_info['statuses'];
			if (array_sum($statuses) != SQ_SC_STATUS_ALL) {
				foreach ($statuses as $i => $status) {
					$statuses[$i] = $db->quote($status);
				}
				$query['where'][] = 'a.status IN ('.implode(', ', $statuses).')';
			}
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$query['where'][] = 'a.status  >= '.$db->quote(SQ_STATUS_LIVE);
		}

		// ASSET TYPE RESTRICTIONS
		if (!empty($search_info['asset_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['asset_types']); NULL !== ($i = key($search_info['asset_types'])); next($search_info['asset_types'])) {
				if ($search_info['asset_types'][$i]) {
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
			$query['where'][] = 'a.type_code IN (
									SELECT type_code
									FROM sq_ast_typ_inhd
									WHERE '.$type_code_cond.'
								)';
		}

		return $query;

	}//end constructBaseSearchQuery()


	/**
	* Combines asset scores according to the passed logic
	*
	* Returns a mapping from assetids to combined scores
	*
	* @param array	$asset_scores1	a mapping from assetids to scores
	* @param array	$asset_scores2	a mapping from assetids to scores
	* @param string	$logic			either 'AND' or 'OR'
	* @param string	$word			word
	*
	* @return array
	* @access private
	*/
	function combineAssetScores($asset_scores1, $asset_scores2, $logic='AND', $word=NULL)
	{
		$result_scores = NULL;
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
					$out_asset_scores[$assetid]['search_score'] = $out_asset_scores[$assetid]['search_score'] + $asset_scores2[$assetid]['search_score'];
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
	* @param string	$word	The word to compare
	*
	* @return boolean
	* @access public
	*/
	function isNoiseWord($word)
	{
		$words = $this->attr('noise_word_list');
		foreach ($words as $key => $value) {
			if (strcasecmp($key, $word) == 0) return TRUE;
		}
		return FALSE;

	}//end isNoiseWord()


	/**
	* Return true if word is in white words
	*
	* @param string	$word	The word to compare
	*
	* @return boolean
	* @access public
	*/
	function isWhiteWord($word)
	{
		$words = $this->attr('white_word_list');
		foreach ($words as $key => $value) {
			if (strcasecmp($key, $word) == 0) return TRUE;
		}
		return FALSE;

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


	/**
	* Gets the appropriate plugin for the database engine
	*
	* Database plugins live in the plugins/[db-type] sub-folder of the Search
	* Manager and have a class name of Search_Manager_Plugin_[db-type]
	*
	* @return object
	* @access private
	*/
	function &_getDBPlugin()
	{
		if (is_null($this->_db_plugin)) {
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$db_type = $db->phptype;

			$dir = dirname(__FILE__).'/plugins/'.strtolower($db_type);
			$class_name = 'search_manager_plugin_'.$db_type;
			require_once $dir.'/'.$class_name.'.inc';
			$this->_db_plugin =& new $class_name();
		}

		return $this->_db_plugin;

	}//end _getDBPlugin()


	/**
	* Provide generic format of search results data
	*
	* Results array is in the format returned by search manager
	* when a detailed searched is made
	*
	* @param array	$results		Search Results
	* @param string	$search_string	String searched for
	*
	* @return array
	* @access public
	*/
	function getFormattedResults($results, $search_string)
	{
		$search_string = preg_replace('/([\.\\\!\+\*\?\[\]\^\$\(\)\=\!\<\>\|\:]+)/i', '\\\${1}',$search_string);

		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$formatted_results = Array();

		// transform the array into a better working format
		foreach ($results as $result) {
			$searched_results[$result['assetid']][] = $result['source'];
		}

		// populate the results array as required
		// note we dont include fields that dont have matches
		foreach ($searched_results as $assetid => $content) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			foreach ($content as $content_type) {
				switch ($content_type['source']) {

					// asset contents containing search string
					case preg_match('/contents/', $content_type) > 0:
						$asset_contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($assetid);

						foreach ($asset_contents as $contents_id => $contents_value) {
							if (preg_match('/'.htmlentities($search_string).'/i', $contents_value) || preg_match('/'.($search_string).'/i', $contents_value)) {
								$formatted_results[$assetid]['contents'][$contents_id] = $contents_value;
							}
						}
					break;

					// attributes containing search string
					// attributes come in 2 formats, __attribute__ and attr:attrname$
					case preg_match('/__(.+)__/', $content_type, $matches) > 0:
					case preg_match('/attr:(.+)$/', $content_type, $matches) > 0:
						if (isset($asset->vars[$matches[1]])) {
							$attribute = $asset->attr($matches[1]);

							if (preg_match("/$search_string/i", $attribute)) {
								$formatted_results[$assetid]['attributes'][$matches[1]] = $attribute;
							}
						}
					break;

					// metadata containing search string
					case preg_match('/metadata/', $content_type) > 0:
						// we've already set metadata for this asset
						if (isset($formatted_results[$assetid]['metadata'])) {
							break;
						}

						$metadata = $mm->getMetadata($assetid);

						foreach ($metadata as $fieldid => $data) {
							if (preg_match("/$search_string/i", $data['value'])) {
								$formatted_results[$assetid]['metadata'][$fieldid] = $data;
							}
						}

						// if no metadata was set, then we were looking for the schema
						if (!isset($formatted_results[$assetid]['metadata'])) {
							$schemas = $mm->getSchemas($assetid, TRUE);

							foreach ($schemas as  $schemaid) {
								$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
								$schema_data = $mm->getSchemaDefaultValues($schema->id);

								foreach ($schema_data as $fieldid => $data) {
									if (preg_match("/$search_string/i", $data['value'])) {
										$formatted_results[$schema->id]['schema'][$fieldid] = $data;
									}
								}

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($schema);
							}
						}
					break;

					default:
					break;

				}//end switch content type
			}//end foreach $assetid

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

		}//end foreach $results

		return $formatted_results;

	}//end getFormattedResults()


}//end class

?>
