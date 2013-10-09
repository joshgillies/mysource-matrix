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
* $Id: search_manager.inc,v 1.257 2013/10/09 05:09:42 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types.
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.257 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);
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
	function create(Array &$link)
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
		$this->reindexAttributes($broadcaster, $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($broadcaster->type(), Array('name')), TRUE);

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


	/**
	* Event fired when a context gets deleted.
	*
	* We want to flush any key data in the index table related to that
	* context. This is necessary because the values may linger in the table,
	* and cause key duplication issues.
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								Context deleted is set as $vars['contextid']
	*
	* @return void
	* @access public
	*/
	function onContextDelete(&$broadcaster, $vars=Array())
	{
		if (!$this->attr('indexing')) return FALSE;
		MatrixDAL::executeQuery('search_manager', 'deleteDeadContextIndexableContent', $vars);

	}//end onContextDelete()


//--        ASSET INDEXING        --//


	/**
	* Reindex this asset and, if specified, the metadata associated with this asset
	*
	* No multi-context support is required here, as standard asset fields and metadata
	* do not vary based on context.
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

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end reindexAsset()


	/**
	* Reindex the specified asset's attributes
	*
	* With multi-context searching, extra parameters have been added.
	* The $all_contexts flag will be set to TRUE when a "all contexts reindex"
	* is necessary. This will be when the Reindex HIPO job is called, or when
	* the asset is first created.
	*
	* @param object		&$asset			the asset to reindex
	* @param array		$vars			a list of attribute names that have
	*									changed
	* @param boolean	$all_contexts	TRUE = attributes are reindexed in
	*									all contexts. FALSE = only current
	*									context is reindexed.
	*
	* @return void
	* @access public
	*/
	function reindexAttributes(&$asset, $vars=Array(), $all_contexts=FALSE)
	{
		// if search manager is reindexed then there is infinite recursion!!!
		if ($asset->id == $this->id) return FALSE;

		// We can't reindex attributes if this asset does not have any attributes
		if (empty($asset->vars)) return FALSE;

		$contextid = (int)$GLOBALS['SQ_SYSTEM']->getContextId();
		$all_contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());

		// only delete the attributes that have changed
		$flush_vars = Array();
		if (!in_array('all', $vars)) {
			$flush_vars = $vars;
		} else {
			// flush all the indexable attributes for this asset type
			$flush_vars = array_keys($this->getIndexableAttributes($asset->type()));
		}

		if ($all_contexts === TRUE) {
			$contexts_todo = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
		} else {
			$contexts_todo = Array($contextid);
		}

		if (count($flush_vars) > 0) {
			foreach ($flush_vars as &$flush_var) {
				$flush_var = 'attr:'.$flush_var;
			}
			unset($flush_var);
		}

		$all_indexable_content = Array();

		if ($this->isAssetIndexable($asset->id) === FALSE) {
			// Asset is not indexable.
			// Flush the vars in all passed contexts, plus default
			$this->flushIndexableContent($asset->id, $flush_vars, array_merge($all_contextids, Array('default')));
		} else {
			// Asset is indexable. Yay.
			$indexable_content = $this->getIndexableAttributeContent($asset, $vars, $all_contexts);

			if (!empty($indexable_content)) {
				if ($all_contexts === TRUE) {
					// If all contexts, we'll consider it a fresh start.
					$this->flushIndexableContent($asset->id, $flush_vars, array_merge($all_contextids, Array('default')));
				} else {
					if ($contextid === 0) {
						// Flush out all the default values for these
						$this->flushDefaultIndexableContent($asset->id, $flush_vars);
					} else {
						// We only have to flush the indexable content for this context,
						// because we are only adding values for this context after all,
						// which won't be defaults
						$this->flushIndexableContent($asset->id, $flush_vars, Array($contextid));

						// Also flush the non-contextable attributes updated in this context
						// This will be indexed as the general "default context" value (see bug #5832)
						$flush_non_contextable_vars = Array();
						foreach($indexable_content as $attr_idx_data) {
							if (isset($attr_idx_data['contextid']) && $attr_idx_data['contextid'] == 'default' && isset($attr_idx_data['component'])) {
								$flush_non_contextable_vars[$attr_idx_data['component']] = 1;
							}//end if
						}//end foreach
						if (!empty($flush_non_contextable_vars)) {
							$this->flushIndexableContent($asset->id, array_keys($flush_non_contextable_vars), Array('default'));
						}
					}
				}

				// Indexable content includes contextid/use default information. Now
				// we can add it.
				$this->addIndexableContent($asset->id, $indexable_content);
			}
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end reindexAttributes()


	/**
	* Reindex the specified asset's custom contents
	*
	* With multi-context searching, extra parameters have been added.
	* The $all_contexts flag will be set to TRUE when a "all contexts reindex"
	* is necessary. This will be when the Reindex HIPO job is called. (Contents
	* are not reindexed when the asset is first created.)
	*
	* @param object		&$asset			the asset to reindex
	* @param array		$vars			a list of attribute names that have
	*									changed
	* @param boolean	$all_contexts	If TRUE, the asset contents wil be
	*									reindexed from all contexts' perspective.
	*									FALSE = only current context.
	*
	* @return void
	* @access public
	*/
	function reindexContents(&$asset, $vars=Array(), $all_contexts=FALSE)
	{
		// Find the asset that we want to index the content under.
		// this will be the top most dependant asset above this asset
		if (!$indexing_assetid = $this->getIndexingAssetid($asset)) {
			return FALSE;
		}
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		$indexing_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($indexing_assetid);

		// check that we want to index this asset first
		if (!$this->isAssetIndexable($indexing_assetid)) {
			return;
		}

		if ($all_contexts === TRUE) {
			$contexts_todo = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
		} else {
			$contexts_todo = Array($contextid);
		}

		$this->flushIndexableContent($indexing_assetid, Array('__contents__'), $contexts_todo);

		foreach ($contexts_todo as $processed_contextid) {
			$other_context = FALSE;
			if (((int)$contextid === (int)$processed_contextid)) {
				// Same context as was passed, OR, asset is being created
				// and therefore all context values will be the same
				$contexted_asset =& $indexing_asset;
			} else {
				$other_context = TRUE;
				// Grab the contexted version of the asset
				$GLOBALS['SQ_SYSTEM']->changeContext($processed_contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
			}

			$index_content  = $this->getIndexableContent($contexted_asset);

			foreach ($index_content as &$index_content_item) {
				$index_content_item['contextid'] = $processed_contextid;
			}
			$this->addIndexableContent($indexing_assetid, $index_content);

			if ($other_context === TRUE) {
				unset($contexted_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}
		}

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
		$array_keys = array_keys($parents);
		$parent = array_shift($array_keys);

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

			$name = ($name_type == 'name') ? $asset->name : $asset->short_name;
			$index_content = array_merge($index_content, $plugin->splitIndexableContent($name, $type_code, 'text', $name_index, $weightings[$name_index], 'default', TRUE));
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
								'contextid'	=> 'default',
								'use_default'	=> '1',
							   );
		}

		if (isset($weightings['__assetid__'])) {

			if (in_array('assetid', $changed_vars) || $reindex_all) {

				$index_content[] = Array(
									'value'		=> $asset->id,
									'type_code'	=> $type_code,
									'type'		=> 'int',
									'component'	=> '__assetid__',
									'score'		=> $weightings['__assetid__'],
									'contextid'	=> 'default',
									'use_default'	=> '1',
								   );
			}
		}

		foreach ($index_content as &$index_content_item) {
			$index_content_item['contextid'] = NULL;
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
	function getIndexableAttributeContent(&$asset, $changed_vars, $all_contexts=TRUE)
	{
		$plugin        =& $this->_getDBPlugin();
		$weightings    =  $this->getWeightings($asset->id);
		$index_content =  Array();
		$type_code     =  $asset->type();
		$reindex_all   =  in_array('all', $changed_vars);
		$contextid     =  $GLOBALS['SQ_SYSTEM']->getContextId();

		if ($all_contexts === TRUE) {
			$asset_vars = Array();
			$context_list = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
			foreach ($context_list as $this_contextid) {
				if ($this_contextid !== $contextid) {
					$GLOBALS['SQ_SYSTEM']->changeContext($this_contextid);
					$context_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
				} else {
					$context_asset = $asset;
				}

				$asset_vars[$this_contextid] = $context_asset->vars;

				if ($this_contextid !== $contextid) {
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($context_asset->id);
					unset($context_asset);
					$GLOBALS['SQ_SYSTEM']->restoreContext();
				}
			}
		} else {
			$context_list = Array();
			// We might need the "default" context values for non-contextable attributes
			if ($contextid !== 0) {
				$context_list[] = 0;
				$GLOBALS['SQ_SYSTEM']->changeContext(0);
				$context_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id);
				$asset_vars[0] = $context_asset->vars;
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($context_asset->id);
				unset($context_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}
			$context_list[] = $contextid;
			$asset_vars[$contextid] = $asset->vars;
		}

		// We cannot use isset() because we have to handle the possibility of a
		// NULL default value, so will try and fudge with an array and
		// array_key_exists(). Hold onto your hats, folks...
		$default_value = Array();
		foreach ($asset->vars as $var_name => $var_data) {

			// only reindex the attributes that have been changed
			if (!in_array($var_name, $changed_vars) && !$reindex_all) {
				continue;
			}

			// if there is no weightings for this attribute, then its not indexable
			if (!isset($weightings[$var_name])) continue;

			// don't index passwords or serialise attributes
			if ($var_data['type'] == 'password' || $var_data['type'] == 'serialise') {
				continue;
			}

			$attrid = $var_data['attrid'];
			$values = Array();
			if ((int)$var_data['is_contextable'] === 0) {
				$attr = $GLOBALS['SQ_SYSTEM']->am->getAttribute($attrid);
				$attr->setValue($asset_vars[0][$var_name]['value']);
				$values['default'] = $attr->getContent();
			} else {
				foreach (array_keys($asset_vars) as $this_contextid) {
					$attr = $GLOBALS['SQ_SYSTEM']->am->getAttribute($attrid);
					$attr->setValue($asset_vars[$this_contextid][$var_name]['value']);
					// Need to check if the 'use_default' key is set to NULL
					if (array_key_exists('use_default', $asset_vars[$this_contextid][$var_name]) && (int)$asset_vars[$this_contextid][$var_name]['use_default'] === 1) {
						// Is the default value for this attribute found yet?
						// Otherwise, don't bother reloading the attribute.
						if (isset($default_value[$var_name]) === FALSE) {
							$default_value[$var_name] = $attr->getContent();
						}

					} else {
						$values[$this_contextid] = $attr->getContent();
					}
				}
			}

			if ((int)$var_data['is_contextable'] === 0) {
				$list_to_test = Array('default');
			} else {
				$list_to_test = $context_list;
			}

			foreach ($list_to_test as $this_contextid) {
				if ($this_contextid === 'default') {
					// Non-contextable attribute.
					$use_default = TRUE;
				} else {
					$use_default = ($this_contextid === 0 ? TRUE : FALSE);
					if (array_key_exists($this_contextid, $values) === FALSE) {
						$values[$this_contextid] = $default_value[$var_name];
						$use_default = TRUE;
					}
				}

				$value = $values[$this_contextid];

				//strip the formatting tags first because they should not be replace with space (' '), e.g. <b>H</b>ello => Hello
				$formatting_tags = Array('span', 'b', 'big', 'em', 'i', 'small', 'strong', 'sub', 'sup', 'ins', 'del');
				$value = $this->_strip_tags($formatting_tags, $value);
				// strip tags and replace with whitespace so as to preserve word boundaries
				$value = preg_replace('/<\/?[^>]+>/i', ' ', $value);
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
										'value'			=> $value,
										'type_code'		=> $type_code,
										'type'			=> $var_data['type'],
										'component'		=> $comp_name,
										'score'			=> $weightings[$var_name],
										'contextid'		=> $this_contextid,
										'use_default'	=> $use_default,
									   );
				} else {
					$index_content = array_merge($index_content, $plugin->splitIndexableContent($value, $type_code, $var_data['type'], $comp_name, $weightings[$var_name], $this_contextid, $use_default));
				}
			}//end foreach context in value list

		}//end foreach

		unset($default_value);

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
		$contextid     =  $GLOBALS['SQ_SYSTEM']->getContextId();

		// only index this if there is a weighting set
		if (isset($weightings['__contents__'])) {

			// ask the asset for its content, and store it under a contents component
			// we don't want to index any tags so strip them

			$contents = $asset->getContent();

			//strip the formatting tags first because they should not be replace with space (' '), e.g. <b>H</b>ello => Hello
			$formatting_tags = Array('span', 'b', 'big', 'em', 'i', 'small', 'strong', 'sub', 'sup', 'ins', 'del');
			$contents = $this->_strip_tags($formatting_tags, $contents);
			// strip tags and replace with whitespace so as to preserve word boundaries
			$contents = preg_replace('/<\/?[^>]+>/i', ' ', $contents);

			if (trim($contents) != '') {
				$index_content = $plugin->splitIndexableContent($contents, $type_code, 'text', '__contents__', $weightings['__contents__'], $contextid, FALSE);
			}
		}

		return $index_content;

	}//end getIndexableContent()


	/**
	* Adds the specified index content for the specified assetid to the indexing system
	*
	* @param string		$assetid		the assetid to index the content under
	* @param array		&$index_content	the content to index
	*
	* @return boolean
	* @access public
	*/
	function addIndexableContent($assetid, &$index_content)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return FALSE;
		if (!is_array($index_content)) return FALSE;

		$index_content_found = FALSE;
		$keys = array_keys($index_content);

		// Array to keep track of which context we are using for a certain
		// assetid/component combination. We must track this separately; to use
		// $defaults for this would result in only one item of split content
		// to be indexed.
		$defaults_context = Array();

		// List of values that will be added at the end using the 'use default'
		// insert, after all custom values are entered.
		$defaults = Array();
		$add_content = Array();
		// Pre-pack the custom indexable content query, so it doesn't have to
		// prepare itself over and over again
		if (!empty($keys)) {
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

					//wysiwyg type would have the same characteristics to text type, e.g. a value which is longer than 255 characters will not be indexed
					case 'metadata_field_wysiwyg':
					case 'wysiwyg':
					case 'text':
						$value = $this->_getRawNumberInText($value);
						// filter noise words
						if (!$this->isWordIndexable($value)) continue 2; // continue the foreach loop
					break;

					default:
						// Selection and thesaurus types do not need massaging.
						// (splitIndexableContent() in DB plugin lowercases words
						// already if required)
					break;
				}

				// work out whether we've already added the default for this
				$default_key = $assetid.' '.$data['component'].' '.sha1($value);

				$bind_vars = Array(
								'value'			=> $value,
								'assetid'		=> $assetid,
								'type_code'		=> $data['type_code'],
								'component'		=> $data['component'],
								'contextid'		=> ($data['contextid'] === NULL) ? 'default' : (string)$data['contextid'],
								'type'			=> $data['type'],
								'score'			=> $data['score'],
								'use_default'	=> $data['use_default'] ? '1' : '0',
							 );

				// Save up the default values so we can put them in after custom
				// values... but only if we have not seen them before (lock one
				// context as the one we saw first, and only accept values from it)
				if (((int)$bind_vars['use_default'] === 1) && ($bind_vars['contextid'] !== 'default')) {
					if (array_key_exists($default_key, $defaults_context) === FALSE) {
						$defaults_context[$default_key] = $bind_vars['contextid'];
					}
					if ($bind_vars['contextid'] === $defaults_context[$default_key]) {
						$defaults[] = $bind_vars;
					}
				} else {
					// Don't perform database transactions here as it can fail silently here without warning
					$add_content[] = $bind_vars;
				}

			}//end foreach
		}//end if

		// This could be out of a transaction at this point, and in db1 (the
		// read-only db). Change to db2, and wrap in a transaction so it doesn't
		// autocommit all over the place
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		// And finally, add the indexable content
		// This code was moved outside the loop below which creates both the default and now the content.
		// This was due to a bug in PHP (<5.2) and Postgres 8.1 dropping transactions silently
		// when a prepared statement is not executed.

		if (!empty($add_content)) {
			$dal_query = MatrixDAL::getQuery('search_manager', 'addIndexableContent');
			$query = $dal_query->prepare();

			foreach ($add_content as $bind_vars) {

				try {
					// Non-default value, or non-contextable. Put it in straight away.
					foreach ($bind_vars as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
					}
					MatrixDAL::execPdoQuery($query);
					$index_content_found = TRUE;
				} catch (Exception $e) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					throw new Exception('Unable to add indexable content due to database error: '.$e->getMessage());
				}
			}//end foreach
		}//end if

		// Okay, shove in the default values, which will be determined by what
		// context values are not already in there.
		// Pre-pack the query first though, so it doesn't potentially get
		// prepared 1000's of times over.
		if (!empty($defaults)) {
			if (DAL::getDbType() === 'oci') {

				// For Oracle system, we'll use two queries to add content into search index table
				// First query to the get the contextids which doesn't have context values already there
				// and second to insert the contextid along with other indexable content into index table
				// See bug #4136
				$dal_get_context_query = MatrixDAL::getQuery('search_manager', 'getDefaultContext');
				$get_context_query = $dal_get_context_query->prepare();

				$dal_insert_query = MatrixDAL::getQuery('search_manager', 'addIndexableContent');
				$insert_query = $dal_insert_query->prepare();


				foreach ($defaults as $bind_vars) {
					try {
						MatrixDAL::bindValueToPdo($get_context_query, 'assetid', $bind_vars['assetid']);
						MatrixDAL::bindValueToPdo($get_context_query, 'component', $bind_vars['component']);
						$result = MatrixDAL::executePdoAll($get_context_query);

						foreach($result as $asset_context) {
							$bind_vars['contextid'] = $asset_context['contextid'];
							foreach ($bind_vars as $bind_var => $bind_value) {
									MatrixDAL::bindValueToPdo($insert_query, $bind_var, $bind_value);
							}
							MatrixDAL::execPdoQuery($insert_query);
						}

						$index_content_found = TRUE;
					} catch (Exception $e) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						throw new Exception('Unable to add indexable content due to database error: '.$e->getMessage());
					}
				}//end foreach
			} else {

				$dal_query = MatrixDAL::getQuery('search_manager', 'addDefaultIndexableContent');
				$query = $dal_query->prepare();

				foreach ($defaults as $bind_vars) {
					try {
						foreach ($bind_vars as $bind_var => $bind_value) {
							if ($bind_var !== 'contextid') {
								MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
							}
						}
						MatrixDAL::bindValueToPdo($query, 'assetid_1', $bind_vars['assetid']);
						MatrixDAL::bindValueToPdo($query, 'component_1', $bind_vars['component']);
						MatrixDAL::bindValueToPdo($query, 'value_1', $bind_vars['value']);
						MatrixDAL::execPdoQuery($query);

						$index_content_found = TRUE;
					} catch (Exception $e) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						throw new Exception('Unable to add indexable content due to database error: '.$e->getMessage());
					}
				}
			}//end else

		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $index_content_found;

	}//end addIndexableContent()


	/**
	* Flush the keydata for an asset
	*
	* If the components argument is provided, only those components will be
	* flushed. Otherwise, all components of that asset will be deleted from
	* the search index.
	*
	* @param int	$assetid	ID of the asset
	* @param array	$components	the components to which the keydata pertains (e.g. metadata or attribute)
	* @param array	$contextids	Array of context IDs to flush key data.
	*							If omitted, all contexts (including defaults)
	*							will be flushed.
	*
	* @return void
	* @access public
	*/
	function flushIndexableContent($assetid, $components=Array(), $contextids=NULL)
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return;

		// This could be out of a transaction at this point, and in db1 (the
		// read-only db). Change to db2, and wrap in a transaction so it doesn't
		// autocommit all over the place
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			if ($contextids === NULL) {
				$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
				$contextids[] = 'default';
			}

			if (is_array($contextids) === FALSE) {
				$contextids = Array($contextids);
			}


			$bind_vars = Array(
							'assetid'		=> $assetid,
							'contextids'	=> $contextids,
						 );

			if (!empty($components)) {
				$bind_vars['components'] = $components;
			} else {
				$bind_vars['components'] = NULL;
			}

			$affected_rows = MatrixDAL::executeQuery('search_manager', 'flushIndexableContent', $bind_vars);
		} catch (DALException $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			throw new Exception('Unable to flush indexable content due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end flushIndexableContent()


	/**
	* Flush all 'default' indexed data for an asset
	*
	* Default indexed data are values indexed from a non-contextable attribute,
	* or a contextable attribute using the default value.
	*
	* If the components argument is provided, only those components will be
	* flushed. Otherwise, all components of that asset will be deleted from
	* the search index.
	*
	* @param int	$assetid	ID of the asset
	* @param array	$components	the components to which the keydata pertains (e.g. metadata or attribute)
	* @param array	$contextids	Array of context IDs to flush key data.
	*							If omitted, all contexts (including defaults)
	*							will be flushed.
	*
	* @return void
	* @access public
	*/
	function flushDefaultIndexableContent($assetid, $components=Array())
	{
		// don't run if indexing turned OFF
		if (!$this->attr('indexing')) return;

		// This could be out of a transaction at this point, and in db1 (the
		// read-only db). Change to db2, and wrap in a transaction so it doesn't
		// autocommit all over the place
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars = Array(
							'assetid'		=> $assetid,
							'use_default'	=> '1',
						 );

			if (!empty($components)) {
				$bind_vars['components'] = $components;
			} else {
				$bind_vars['components'] = NULL;
			}

			$affected_rows = MatrixDAL::executeQuery('search_manager', 'flushDefaultIndexableContent', $bind_vars);
		} catch (DALException $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			throw new Exception('Unable to flush default indexable content due to database error: '.$e->getMessage());
		}

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
		$db = MatrixDAL::getDb();

		$sql = 'SELECT
					component
				FROM
					sq_sch_idx
				WHERE
					assetid = :assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			$components = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get indexed components due to database error: '.$e->getMessage());
		}

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
		//check asset weight first because it has highest priority (asset weight > asset tree weight > global weight)
		$weights = $this->getAssetWeightings($assetid);

		if (empty($weights)) {
			//check asset tree weight and global weight (because global weight is also a type of tree weight with the treeid is root id (usually has assetid 1))
			$weights = $this->getAssetTreeWeightings($assetid, FALSE);
		}

		// if there is no indexed value in the array then we can assume
		// that indexing is on for this asset
		return (!isset($weights['indexed']) || $weights['indexed']);

	}//end isAssetIndexable()


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
		if (!$this->attr('indexing')) return FALSE;
		// We've been told not to do this right now
		if ($broadcaster->shouldFastTrack('search_manager_reindex_metadata')) return FALSE;

		$this->reindexMetadata($broadcaster->id, $vars, TRUE);

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
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$fieldids = array_keys($mm->getMetadataFields(Array($vars['schemaid'])));
		for (reset($fieldids); NULL !== ($key = key($fieldids)); next($fieldids)) {
			$fieldids[$key] = 'metadata:'.$fieldids[$key];
		}

		$this->flushIndexableContent($broadcaster->id, $fieldids);

	}//end onMetadataDeleted()


	/**
	* Re-indexes the metadata components for the specified asset
	*
	* @param string		$assetid		the assetid to reindex
	* @param array		$vars			the schemas to renidex
	* @param boolean    $all_contexts   TRUE = attributes are reindexed in all contexts. FALSE = only current context is reindexed.
	*
	* @return void
	* @access public
	*/
	function reindexMetadata($assetid, $vars=Array(), $all_contexts=FALSE)
	{
		assert_valid_assetid($assetid);
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		$contextid = (int)$GLOBALS['SQ_SYSTEM']->getContextId();
        $all_contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());

		if (in_array('all', $vars)) {
			$schema_ids = array_keys($mm->getSchemas($assetid));
			$fieldids = $mm->getMetadataFields($schema_ids);
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
			if ($all_contexts) {
				$this->flushIndexableContent($assetid, $flush_fieldids);
			} else {
				if ($contextid === 0) {
					$this->flushDefaultIndexableContent($assetid, $flush_fieldids);
				} else {
					$this->flushIndexableContent($assetid, $flush_fieldids, Array($contextid));
				}
			}
		}

		if (!$this->isAssetIndexable($assetid)) return;

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		$indexable = $this->getIndexableMetadataContent($asset, $fieldids, $all_contexts);

		$this->addIndexableContent($assetid, $indexable);

	}//end reindexMetadata()


	/**
	* Returns the indexable metadata content for the specified asset
	*
	* @param object	&$asset		The asset to get the indexable content fors
	* @param array	$fieldids	Array of fieldid => type_code to limit to
	* @param boolean    $all_contexts   TRUE = attributes are reindexed in all contexts. FALSE = only current context is reindexed
	*
	* @return array
	* @access public
	*/
	function getIndexableMetadataContent(&$asset, $fieldids=Array(), $all_contexts=TRUE)
	{
		$index_content = Array();
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$plugin =& $this->_getDBPlugin();
		$type_code = $asset->type();

		if (is_null($asset)) return $index_content;

        // Get the current contextid
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$context_list = $all_contexts ? array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts()) : Array($contextid);

		if (empty($fieldids)) {
			$fieldids = $mm->getMetadataFields($mm->getSchemas($asset->id, TRUE));
		}

		// Get all metadata for each context in advance. This is quicker
		// than getting it for each context for each field later..
		$all_metadata = Array();
		foreach ($context_list as $c) {
			$all_metadata[$c] = $mm->getMetadata($asset->id, 0, $c);
		}

		foreach ($fieldids as $fieldid => $field_type) {

			$flag_match_found = FALSE;
			$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($fieldid, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'minor', NULL, TRUE);

			foreach ($current_links as $link) {
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
				$restrictions = $section->attr('restrict');
				if (!empty($restrictions)) {
					foreach($restrictions as $type_code_restricted => $inherit_it) {
						if ($inherit_it && !$flag_match_found) {
							$asset_type_with_parents = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo(Array($asset->id));
							foreach($asset_type_with_parents[$asset->id] as $index => $asset_type) {
								if (array_key_exists($asset_type[0], $restrictions) && !$flag_match_found) {
									$flag_match_found = TRUE;
								}
							}
						} else {
							if (array_key_exists($asset->type(), $restrictions)) {
								$flag_match_found = TRUE;
							}
						}
					}
				} else {
					// There is at least one section without any restrictions where this field is linked to
					$flag_match_found = TRUE;
				}

				// No need to check futher restrictions on other metadata sections
				if ($flag_match_found) break;
			}

			if (!$flag_match_found) {
				continue;
			}

			// dont index this fieldid if getMetadataWeighting return false
			if (FALSE === ($weighting = $this->getMetadataWeighting($asset->id, $fieldid))) {
				continue;
			}

			// new condition for 3.18 because of the result format changes
			if (is_array($field_type)) {
				$field_type = $field_type[0]['type_code'];
			}//end if
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid, $field_type);

			// If metadata field is non-contextable then it means it will have same value for all contexts
			$is_contextable = (int)$field->attr('is_contextable') === 0 ? FALSE : TRUE;
			$context_to_index = $is_contextable ? $context_list : Array($context_list[0]);

			foreach($context_to_index as $this_contextid) {

				// Get metadata for this context (here's one we prepared earlier)
				$metadata = $all_metadata[$this_contextid];

				if (isset($metadata[$field->id][0]['value'])) {
					$value = $metadata[$field->id][0]['value'];
				} else if ($field instanceof Metadata_Field_WYSIWYG) {
					$value = $field->attr('default_html');
				} else {
					$value = $field->attr('default');
				}

         		$use_default = empty($value) || $this_contextid === 'default' || $this_contextid === 0 ? TRUE : FALSE;

				$value_components = Array();
				Metadata_Field::decodeValueString($value, $value, $value_components);

				for($i = 0; $i < 3; $i++) { //attempt to replace nested keywords max 3 levels. See Bug #5295
					$keywords = retrieve_keywords_replacements($value, '.');

					$the_replacement = $mm->generateKeywordReplacements($asset, $keywords, FALSE);

					foreach($keywords as $keyword) {

						if (!isset($the_replacement[$keyword])) {
							// Looking for further replacements
							$the_replacement[$keyword] = str_replace('"', '\"', $asset->getKeywordReplacement($keyword));

						} else if ($the_replacement[$keyword] == '%'.$keyword.'%' && substr($keyword, 0, 15) == 'metadata_field_') {

							 // Remove any modifiers from keyword
							$full_keyword = $keyword;
							$keyword = parse_keyword($keyword, $modifiers);

							// searching for keywords that replace the content of another field
							$fieldid = $mm->getFieldAssetIdFromName($asset->id, substr($keyword, 15));
							$keyword_replacement = $mm->getMetadataValueByAssetid($asset->id, $fieldid, TRUE, FALSE);

							if ($keyword_replacement !== NULL) {
								if (count($modifiers) > 0) {
									apply_keyword_modifiers($keyword_replacement, $modifiers, Array('assetid' => $asset->id));
								}
								$the_replacement[$full_keyword] = $keyword_replacement;
							} else {
								$the_replacement[$full_keyword] = '%'.$full_keyword.'%';
							}

						}//end if

					}//end foreach

					replace_keywords($value, $the_replacement);
				}//end for

				// check field_type to define the indexing way
				if ($field_type == 'metadata_field_date') {
					$index_content[] = Array(
										'value'		=> $value,
										'type_code'	=> $type_code,
										'type'		=> 'datetime',
										'component'	=> 'metadata:'.$field->id,
										'score'		=> $weighting,
										'contextid'	=> $this_contextid,
										'use_default'	=> $use_default,
									   );
				} else {

					switch ($field_type) {
						case 'metadata_field_hierarchy':
							$key_type = 'selection';
						break;

						case 'metadata_field_select':
							if ($field->attr('multiple')) {
								$plugin =& $this->_getDBPlugin();
								$value = $plugin->handleMultipleMetadataSelect($value);
							}

							$key_type = 'selection';
						break;

						case 'metadata_field_multiple_text':
							$plugin =& $this->_getDBPlugin();
							$value = $plugin->handleMultipleMetadataSelect($value);
							$key_type = 'selection';
						break;

						case 'metadata_field_thesaurus':
							$key_type = 'thesaurus';
							// Replace comma with spaces so the values can be indexed correctly. This should be done here and not in get_word_counts/remove_silent_chars.
							$value = str_replace(',',' ', $value);
						break;

						case 'metadata_field_wysiwyg':
							$key_type = 'metadata_field_wysiwyg';
							$plugin =& $this->_getDBPlugin();
							$value = $plugin->handleMultipleMetadataSelect($value);
						break;

						default:
							$key_type = 'text';
							$value = $this->_getRawNumberInText($value);
					}

					$index_content = array_merge($index_content, $plugin->splitIndexableContent($value, $type_code, $key_type, 'metadata:'.$field->id, $weighting, $this_contextid, $use_default));
				}//end else

			}// end foreach contexts_to_use

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
	* Weightings are now cached inside the search manager, per asset, to improve
	* performance (instead of loading up the cached weightings file four times
	* for a reindex, it only gets loaded once). Cached weightings are unset when
	* the weightings are saved in _saveWeightings().
	*
	* @param string	$assetid	the assetid to obtain the weightings for
	*
	* @return array
	* @access public
	*/
	function getWeightings($assetid)
	{
		if (isset($this->_tmp['asset_weightings'][$assetid]) === FALSE) {

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
						$this->_tmp['asset_weightings'][$assetid] = $weightings;
					}
				}
			}

			// if not yet cached using default values, reorder array and cache
			if (isset($this->_tmp['asset_weightings'][$assetid]) === FALSE) {
				$indexed_weightings = Array();
				// only add the weightings if the individual components are indexed
				foreach (Array('attr_weights', 'asset_weights') as $type) {
					foreach ($weightings[$type] as $component => $info) {
						if ($info['indexed']) {
							$indexed_weightings[$component] = $info['weight'];
						}
					}
				}

				$this->_tmp['asset_weightings'][$assetid] = $indexed_weightings;
			}

		}//end if already cached

		return $this->_tmp['asset_weightings'][$assetid];

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
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

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

		$info      = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), Array(), TRUE, 'type_code');
		$type_code = $info[$assetid];

		$weight_sources = Array(
								// check metadata weightings in asset weight customisation
								0 => Array('function_name' => 'Asset'		, 'params' => Array($assetid)),
								// check metadata weightings in asset type of asset tree customisation
								1 => Array('function_name' => 'AssetTree'	, 'params' => Array($assetid, FALSE, FALSE, TRUE, FALSE)),
								// check generic metadata weightings of asset tree customisation
								2 => Array('function_name' => 'AssetTree'	, 'params' => Array($assetid, FALSE, FALSE, FALSE, FALSE)),
								// check metadata weightings in asset type of global weight customisation
								3 => Array('function_name' => 'AssetType'	, 'params' => Array($type_code)),
								// check generic metadata weightings of global weight customisation
								4 => Array('function_name' => 'AssetTree'	, 'params' => Array($GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder'))),
						  );

		foreach ($weight_sources as $source) {
			$fn = 'get'.$source['function_name'].'Weightings';
			$weightings = call_user_func_array(Array($this, $fn), $source['params']);
			if (!empty($weightings) && isset($weightings['metadata_weights'])) {
				$weight = $this->_getLocalMetadataWeightings($weightings['metadata_weights'], $fieldid, $sectionid, $schemaids);

				// if the weight is false then we don't want to index this field
				if ($weight === FALSE) return FALSE;

				// if the weight is not -1 then this is the weight to use
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
			$type_hier = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, TRUE);
			array_unshift($type_hier, $type_code);
		} else {
			$type_hier = Array($type_code);
		}

		$customised_type_codes = array_keys($type_weights);
		foreach ($type_hier as $type_code) {
			if (in_array($type_code, $customised_type_codes)) {
				return $type_weights[$type_code];
			}
		}

		return Array();

	}//end getAssetTypeWeightings()


	/**
	* Returns the Asset Tree weightings for the specified assetid, or all
	* weightings registered for asset trees if the specified assetid is null
	*
	* @param string		$assetid					the assetid of the wanted tree weightings
	*												or null for all asset tree weightings
	* @param boolean	$strict_assetid				if TRUE only weightings that have been
	*												customised explicitly using the specified assetid
	*												will be returned, otherwise the closest weighting
	*												customisation will be returned
	* @param boolean	$strict_type_code			if TRUE only weightings matching the asset type of the
	*												specified assetid will be returned
	* @param boolean	$type_weights_only			Applies only if $assetid is not NULL and $strict_assetid is FALSE.
	* 												if TRUE only type weights data according to $strict_type_code is returned.
	* 												if FALSE $strict_type_code is ineffective & both type_weights & metadata_weights data is returned.
	* @param boolean	$include_global_weights		if TRUE, the asset tree weights will include the root folder weight which defines global weights.
	*
	* @return array
	* @access public
	* @see saveTypeWeightings()
	* @see getMetadataWeighting()
	*/
	function getAssetTreeWeightings($assetid=NULL, $strict_assetid=TRUE, $strict_type_code=FALSE, $type_weights_only=TRUE, $include_global_weights=TRUE)
	{
		$weightings_file = $this->data_path.'/asset_tree_weightings.inc';
		if (file_exists($weightings_file)) {
			require $weightings_file;

			if (!$include_global_weights) {
				$root_folder_assetid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder');
				if (isset($asset_tree_weightings[$root_folder_assetid])) {
					unset($asset_tree_weightings[$root_folder_assetid]);
				}
			}

			if (!is_null($assetid)) {

				if ($strict_assetid) {
					if (isset($asset_tree_weightings[$assetid])) {
						return $asset_tree_weightings[$assetid];
					}
				} else {
					// there is no explicit weighting customisation for the assetid
					// so do a tree lookup for the closest one. getParents ORDER's by treeid
					// by default so we can be garunteed that the closest assetids are at the top of the array
					$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($assetid);

					if ($type_weights_only){
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

						// find the parent asset types of this type code
						if (!$strict_type_code) {
							$type_hier = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, TRUE);
							// add the current type code so that we can also find
							// weightings for this type, not just its parent types
							array_unshift($type_hier, $type_code);
						} else {
							$type_hier = Array($type_code);
						}

						// Search by parents first; that way we will always find the
						// closest asset to what we are using, otherwise having a tree weight and global weight,
						// the tree weight does not get used when using all asset types
						$customised_assetids = array_keys($asset_tree_weightings);
						foreach (array_keys($parents) as $parentid) {
							if (in_array($parentid, $customised_assetids)) {
								if (isset($asset_tree_weightings[$parentid]['type_weights'])) {
									$customised_type_codes = array_keys($asset_tree_weightings[$parentid]['type_weights']);
									foreach ($type_hier as $parent_type_code) {
										if (in_array($parent_type_code, $customised_type_codes)) {
											return $asset_tree_weightings[$parentid]['type_weights'][$parent_type_code];
										}
									}//end foreach
								}
							}//end if
						}//end foreach
					} else {
						if (isset($asset_tree_weightings[$assetid])) {
							return $asset_tree_weightings[$assetid];
						}

						foreach (array_keys($parents) as $parentid) {
                            if (isset($asset_tree_weightings[$parentid])) {
								return $asset_tree_weightings[$parentid];
							}//end if
						}//end foreach
					}//end else
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

		// Flush the asset weightings cache
		unset($this->_tmp['asset_weightings']);

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


//--        TAG STRIP        --//

	/**
	 * Strip tags by tag name
	 *
	 * @param array 	$tags		the array of tag names, e.g. Array('span', 'b', 'i')
	 * @param string 	$contents	the string that contains tags
	 *
	 * @return string	the result string that has tags removed
	 * @access private
	 */
	function _strip_tags($tags, $contents) {
		if (!is_array($tags)) {
			$tags = Array($tags);
		}

		foreach ($tags as $tag) {
			//a tag will start with <, there can be one / if it is close tag, and then the tag name,
			//to make sure that the tags with tag name are removed, not the tags start with that tag name,
			//e.g. remove <b> tag, not <br> tag. The tag must end with >, but before that, it can have / (e.g. <br/> ) or space followed by
			//other characters (to form attributes <h1 align="center">, or the / itself (<br />) )
			$contents = preg_replace('/<\/?'.$tag.'(\/|(\s+[^>]*))?>/i', '', $contents);
		}

		return $contents;

	}//end _strip_tags() function


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
	* @param array		$search_info		an array of search settings in the form
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

		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'processSearch');

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc'; // will definitely need this later.
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		  /////////////////////////////
		 //  CONSTRUCT CACHING KEY  //
		/////////////////////////////
		// we need the cache manager to cache the results of queries
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
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

		// Add the context to the cache key, because the results will differ
		// depending on the context we are in (since we only search the current one)
		$search_cache_key .= '-ctx'.$contextid;
		$this->_tmp['cache_key'] = $search_cache_key;
		$this->_tmp['cm'] =& $cm;

		  //////////////////////
		 //  PREPARE FIELDS  //
		//////////////////////
		foreach ($search_info['fields'] as $field_name => $field_details) {
			$this->_prepareSearchField($search_info['fields'][$field_name]);
		}

		if (isset($search_info['exclude'])) {
			foreach ($search_info['exclude'] as $field_name => $field_details) {
				$this->_prepareSearchField($search_info['exclude'][$field_name]);
			}//end foreach
		}//end if

		  ////////////////////////////////////////////////////
		 //  HANDLE ANY SHADOW ASSET OR BRIDGE ROOT NODES  //
		////////////////////////////////////////////////////
		if (!empty($search_info['roots'])) {
			$root_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($search_info['roots']);
			$shadow_results = Array();
			foreach ($search_info['roots'] as $i => $assetid) {
				if (array_key_exists($assetid, $root_info)) {
					$root = $root_info[$assetid];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($root['type_code']);
					// assert class implements bridge and contains method
					if (implements_interface($root['type_code'], 'bridge') && is_callable(Array($root['type_code'], 'processSearch'))) {
						$shadow_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						$shadow_results = $this->_combineResults($shadow_results, $shadow_asset->processSearch($search_info, $include_context), array_get_index($search_info, 'root_logic', 'OR'));
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($shadow_asset);
						unset($search_info['roots'][$i]);
						$shadow_processed = TRUE;
					}
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
		$base_query['select'][] = 'ai.assetid';
		$base_query['select'][] = 'SUM(ai.score) as search_score';

		if (!in_array('ai.assetid', array_get_index($base_query, 'group_by', Array()))) {
			$base_query['group_by'][] = 'ai.assetid';
		}

		if ($include_context) {
			$base_query['select'][] = 'ai.component as source';
			$base_query['group_by'][] = 'ai.component';
		}

		$base_query['where'][] = '(ai.contextid = '.MatrixDAL::quote($contextid).' OR ai.contextid = '.MatrixDAL::quote('default').')';

		  //////////////////////////
		 //  PROCESS EACH FIELD  //
		//////////////////////////
		$term_totals = Array();

		// Search fields types
		$search_field_types = Array('search' => $search_info['fields']);
		if (isset($search_info['exclude'])) {
			$search_field_types['exclude'] = $search_info['exclude'];
		}

		// Search result for 'search' and 'exclude' fields
		$search_results = Array();
		foreach($search_field_types as $search_field_type => $search_fields) {
			$final_search_type_results = NULL;

			// As per existing behaviour, differnet exclude fields are always combined using "OR"
			$field_logic = $search_field_type == 'exclude' ? 'OR' : array_get_index($search_info, 'field_logic', 'OR');

			foreach ($search_fields as $field_name => $field_details) {
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
	//THE SILENT CHARACTERS WERE REMOVED BEFORE IN _prepareSearchField() FUNCTION
	//						$info = remove_silent_chars($search_term, strpos(DAL::getDbType(), 'oci') !== FALSE ? TRUE : FALSE);
							$term_results = $this->_processWordSearch($search_term, $data_source, $base_query, $word_logic);

							if (!is_null($term_results)) {
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
					}
					$field_results = $this->_combineResults($field_results, $data_source_results, $data_source_logic);
				}

				$cross_source_results = $all_source_assetids;
				foreach ($all_source_results as $term => $results) {
					$cross_source_results = array_intersect_assoc($cross_source_results, $results);
				}

				if (is_null($field_results)) {
					$field_results = $cross_source_results;
				} else {
					$field_results = $field_results + $cross_source_results;
				}
				// if this search field has its own logic, use it first
				if(isset($field_details['specific_field_logic'])) {
					$final_search_type_results = $this->_combineResults($final_search_type_results, $field_results, $field_details['specific_field_logic']);
				}
				else {
					$final_search_type_results = $this->_combineResults($final_search_type_results, $field_results, $field_logic);
				}

			}//end foreach

			$search_results[$search_field_type] = $final_search_type_results;

		}//end foreach

		// Final search results for 'search' terms
		$final_results = array_get_index($search_results, 'search', NULL);

		  //////////////////////////////
		 //  PROCESS EXCLUDED TERMS  //
		//////////////////////////////
		if (!empty($search_results['exclude'])) {

			foreach (array_keys($search_results['exclude']) as $exclude_assetid) {
				if (isset($final_results[$exclude_assetid])) {
					unset($final_results[$exclude_assetid]);
				}
			}
		}

		// join any results from bridges
		if (!empty($shadow_results)) {
			$final_results = $this->_combineResults($final_results, $shadow_results, array_get_index($search_info, 'root_logic', 'OR'));
		}

		if (is_null($final_results)) {
			// If this is the case then we didn't find anything to actually search on.  Maybe no
			// words were long enough.  Anyway, we should at least break the news in the correct format.
			$final_results = Array();
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

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'processSearch');

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
					$attribute = $GLOBALS['SQ_SYSTEM']->am->getAttribute($attrid);
					if (is_null($attribute)) {
						unset($field_details['data_sources'][$i]);
						continue;
					}
					if (!($attribute instanceof Asset_Attribute_Datetime)) {
						$is_datetime = FALSE;
					}
					if (($attribute instanceof Asset_Attribute_Int) || ($attribute instanceof Asset_Attribute_Float)) {
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
					$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['params']['assetid']);
					if (is_null($metadata_field)) {
						unset($field_details['data_sources'][$i]);
						continue;
					}
					if (!($metadata_field instanceof Metadata_Field_Date)) {
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
			// DONT remove unindexable words (then you cannot search for part of it)
			$words = Array();

			//only use mb_strtolower() function if it is supported
			$mb_support = FALSE;
			if (function_exists('mb_strtolower')) {
				$mb_support = TRUE;
			}

			// Multiple selections are passed in as an array, so filter those out to a string
			$word_list = '';
			if (is_array($field_details['words'])) {
				$count = 0;
				foreach ($field_details['words'] as $selected_word) {
					$count++ ;
					$word_list .= $selected_word;
					// bug fix #4047 Oracle search fails multiple word keys search with select/multiselect
					// if OR logic is used we will need to make sure there is a OR in between the search terms or else oracle will consider it to be AND
					// i.e - "search something like this" is considered to be "search AND something AND like AND this"
					$word_list .= ( DAL::getDbType() === 'oci' && $field_details['word_logic'] === 'OR' && count($field_details['words']) > $count ) ? ' OR '  : ' ';
				}//end foreach
				$word_list = trim($word_list);
			} else {
				$word_list = $field_details['words'];
			}//end if

			foreach ($this->generateWordList($word_list) as $word) {
				if ($mb_support) {
					$words[] = mb_strtolower($word, SQ_CONF_DEFAULT_CHARACTER_SET);
				} else {
					//lowercase the string which has non-ASCII characters by encoding them to HTML entities
					$word = htmlentities($word, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
					$word = strtolower($word);
					$word = html_entity_decode($word, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
					$words[] = $word;
				}
			}

			// our OR's have to strtolower'd :( oracle wont like it, get it back to OR
			if (DAL::getDbType() === 'oci' && $field_details['word_logic'] === 'OR' ) {
				foreach ($words as $index => $word) {
					$words[$index] = str_replace(' or ', ' OR ' , $word);
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
		if ($result === FALSE) {
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
		if ($result === FALSE) {
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
		if ($result === FALSE) {
			$plugin =& $this->_getDBPlugin();
			$result = $plugin->processNumericSearch($this, $numeric_range, $data_source, $base_query);
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
			if(empty($new_results)) return $res_results;
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
		$query_comps = $this->constructBaseSearchQuery($search_info);

		if (empty($query_comps)) return Array();
		$query_comps['select'][] = 'distinct ai.assetid';
		$query_comps['select'][] = '1 as weight';

		$bind_vars = array_get_index($query_comps, 'bind_vars', Array());
		$search_sql = implode_sql($query_comps);
		$query = MatrixDAL::preparePDOQuery($search_sql);
		foreach ($bind_vars as $bind_var => $value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $value);
		}//end foreach

		$search_results = Array();

		try {
			$search_results = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to process basic search due to database error: '.$e->getMessage());
		}

		if (isset($search_info['limit'])) {
			// limiting the search results
			$search_results = array_slice($search_results, 0, $search_info['limit'], TRUE);
		}

		// get results from any shadow asset root nodes
		$root_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($search_info['roots']);

		foreach ($root_info as $assetid => $root) {
			$shadow_results = Array();
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($root['type_code']);

			// assert class implements bridge and contains method
			if (implements_interface($root['type_code'], 'bridge') && is_callable(Array($root['type_code'], 'processBasicSearch'))) {
				$shadow_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
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
	function spellCheckWord($word, $language=SQ_TOOL_SPELL_CHECKER_LANG)
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
	* We end up with a subquery from this method which the calling code
	* then uses to restrict which assets the search index table will look at.
	*
	* We end up with something like this for the subquery:
	* select assetid from sq_ast
	* inner join sq_ast_lnk inner join sq_ast_lnk_tree ...
	* where tree.node like ('starting_tree_id%')
	* and assetid in (
	* select assetid from sq_ast_perm
	* ... check all permissions are ok here.
	* )
	*
	* @param array	$search_info	an array of search settings
	*
	* @return array
	* @access public
	* @see processSearch(), general_occasional::implode_sql
	*/
	function constructBaseSearchQuery($search_info)
	{
		$db = MatrixDAL::getDb();

		/**
		 * This is the main query that is returned.
		 * We only need to specify the from table and alias here
		 * The select fields are worked out by the calling method
		 * based on what is being searched and what it has been set up to return.
		*/
		$query = Array(
					'select'		=> Array(),
					'from'			=> Array('sq_sch_idx ai'),
					'join'			=> Array(),
					'where'			=> Array(),
					'where_joiner'	=> 'AND',
					'order_by'		=> Array(),
				);

		/**
		 * The subquery is the part that restricts the search
		 * based on:
		 * - which part of the asset tree we are looking at
		 * - what type of assets we're searching for
		*/
		$subquery = Array(
					'select'		=> Array('a.assetid'),
					'from'			=> Array('sq_ast a'),
					'join'			=> Array(),
					'where'			=> Array(),
					'where_joiner'	=> 'AND',
					'order_by'		=> Array(),
				);

		/**
		 * This becomes part of the subquery of $subquery at the end
		 * if we are restricting based on roles/groups/permissions.
		*/
		$permission_check_query = Array(
					'select'		=> Array('p.assetid'),
					'from'			=> Array('sq_ast_perm p'),
					'join'			=> Array(),
					'where'			=> Array(),
					'where_joiner'	=> 'AND',
					'order_by'		=> Array(),
				);

		// TREE LOCATIONS
		if (!empty($search_info['roots'])) {
			$root_logic = array_get_index($search_info, 'root_logic', 'OR');

			// get the treeids of our search roots
			try {
				$bind_vars = Array(
								'minorids'	=> $search_info['roots'],
							 );
				$root_tree_ids = MatrixDAL::executeGroupedAssoc('search_manager', 'getSearchRootTreeids', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to root tree id due to database error: '.$e->getMessage());
			}

			if ($root_logic == 'AND') {
				foreach (array_values($search_info['roots']) as $i => $rootid) {
					$subquery['join'][] = 'INNER JOIN sq_ast_lnk l'.$i.' ON l'.$i.'.minorid = a.assetid'; // join the link tree and link tables
					$subquery['join'][] = 'INNER JOIN sq_ast_lnk_tree t'.$i.' ON l'.$i.'.linkid =  t'.$i.'.linkid'; // join them to the asset table

					$treeid = $root_tree_ids[$rootid][0]['treeid'];
					$subquery['where'][] = 't'.$i.'.treeid LIKE '.MatrixDAL::quote($treeid.'%');
				}
			} else {
				$subquery['join'][] = 'INNER JOIN sq_ast_lnk l ON l.minorid = a.assetid';
				$subquery['join'][] = 'INNER JOIN sq_ast_lnk_tree t ON t.linkid = l.linkid';

				$treeid_wheres = Array();
				foreach ($root_tree_ids as $treeid) {
					$treeid_wheres[] = '(t.treeid LIKE '.MatrixDAL::quote($treeid[0]['treeid'].'%').')';
				}
				if (!empty($treeid_wheres)) {
					if (count($treeid_wheres) > 1 ) {
						$subquery['where'][] = '('.implode(' OR ', $treeid_wheres).')';
					} else {
						$subquery['where'][] = implode(' OR ', $treeid_wheres);
					}
				}
			}
		}//end if !empty($search_info['roots'])

		// ACCESS RESTRICTIONS
		$user_restrictions = (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if ($user_restrictions) {
			$no_roles = ((isset($search_info['no_roles_check']) && $search_info['no_roles_check'] == 1) || (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '0')) ? TRUE : FALSE;
			$no_group_access_check = (isset($search_info['no_group_access_check']) && $search_info['no_group_access_check'] == 1) ? TRUE : FALSE;
			if (!$no_roles) {
				$permission_check_query['join'][] = 'LEFT JOIN sq_vw_ast_role r ON (p.userid = r.roleid AND r.assetid=p.assetid)';
			}

			$public_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('public_user');
			$current_user =& $GLOBALS['SQ_SYSTEM']->user;
			if (isset($search_info['search_as_public_user'])) {
				$search_as_public_user = $search_info['search_as_public_user'];
			} else {
				$search_as_public_user = 0;
			}
			$public_user_access = (empty($current_user) || $search_as_public_user);
			if ($public_user_access) {
				if (!$no_group_access_check) {
					$userids = $public_user->getUserGroups();
				}//end if
			} else {
				if (!$no_group_access_check) {
					$userids = $current_user->getUserGroups();
				}//end if
				$userids[] = $current_user->id;
			}

			$userids[] = $public_user->id;

			$userids = array_unique($userids);

			$query['bind_vars'] = Array();
			$bind_vars = Array();
			for (reset($userids); NULL !== ($i = key($userids)); next($userids)) {
				$bind_vars[] = ':user'.$i;
				$query['bind_vars']['user'.$i] = $userids[$i];
			}

			$usrids_str = implode(',', $bind_vars);

			$userid_check = '(
								p.userid IN ('. $usrids_str . ')';
			if (!$no_roles) {
				$userid_check .= ' OR r.userid IN (' . $usrids_str . ')';
			}
			$userid_check .= ') ';
			$permission_check_query['where'][] = $userid_check;

			// If we are searching as public user, we don't want to have group and roles check with permission granted.
			// We want to get all denied access as well. But if we are logged in, then we only want public granted access rows.
			$check_group_role = '';
			if (!$public_user_access) {
				$check_group_role = '(
								(p.permission = '.MatrixDAL::quote(SQ_PERMISSION_READ).'
									AND (
										(p.userid <> '.MatrixDAL::quote($public_user->id);
				if (!$no_roles) {
					if ($public_user_access) {
						$check_group_role .= ' OR ';
					} else {
						$check_group_role .= ' AND ';
					}
					$check_group_role .= ' (r.userid IS NULL OR r.userid <> '.MatrixDAL::quote($public_user->id).')';
				}
				$check_group_role .= ')
					OR (p.userid = '.MatrixDAL::quote($public_user->id).' AND granted = \'1\')';
				if (!$no_roles) {
					$check_group_role .= ' OR (r.userid = '.MatrixDAL::quote($public_user->id).' AND granted = \'1\')';
				}

				$check_group_role .= '
									)
								)
								OR
								(p.permission > '.MatrixDAL::quote(SQ_PERMISSION_READ).' AND p.granted = \'1\')
							)';
			}

			if (!empty($check_group_role)) {
				$permission_check_query['where'][] = $check_group_role;
			}//end if
		}//end if

		// STATUS RESTRICTIONS
		if (!empty($search_info['statuses'])) {
			$statuses = $search_info['statuses'];
			if (array_sum($statuses) != SQ_SC_STATUS_ALL) {
				foreach ($statuses as $i => $status) {
					$statuses[$i] = MatrixDAL::quote($status);
				}
				$subquery['where'][] = 'a.status IN ('.implode(', ', $statuses).')';
			}
		} else {
			// if there are no status restrictions, default to LIVE assets only
			$subquery['where'][] = 'a.status  >= '.MatrixDAL::quote(SQ_STATUS_LIVE);
		}

		// ASSET TYPE RESTRICTIONS
		if (!empty($search_info['asset_types'])) {
			$inherited_types = Array();
			$normal_types = Array();
			for (reset($search_info['asset_types']); NULL !== ($i = key($search_info['asset_types'])); next($search_info['asset_types'])) {
				if ($search_info['asset_types'][$i] == 1) {
					$inherited_types[] = MatrixDAL::quote($i);
				} else {
					$normal_types[] = MatrixDAL::quote($i);
				}
			}

			$type_code_cond = Array();

			// if we have inherited types and/or normal types
			if (!empty($inherited_types)) {
				$type_code_cond[] = 'inhd_type_code IN ('.implode(', ', $inherited_types).')';
				if (!empty($normal_types)) {
					$type_code_cond[] = 'type_code IN ('.implode(', ', $normal_types).')';
				}
				$type_code_cond = implode(' OR ', $type_code_cond);
				$subquery['where'][] = 'a.type_code IN (
											SELECT type_code
											FROM sq_ast_typ_inhd
											WHERE '.$type_code_cond.'
										)';
			} else {
				// if we only got normal type instead we are not using the subquery
				$subquery['where'][] = 'a.type_code IN ('.implode(', ', $normal_types).')';
			}
		}

		/**
		 * Now we've worked out all of the restrictions:
		 * - which area we're searching
		 * - what asset types we're searching for
		 * - permissions checks etc..
		 * put it all together into a big subquery.
		 * This then becomes part of the main query where clause.
		*/
		$subquery_string = 'SELECT ' . implode(',', $subquery['select']) . ' FROM ' . implode(',', $subquery['from']) . ' ' . implode(' ', $subquery['join']);
		if (!empty($subquery['where'])) {
			$subquery_string .= ' WHERE ' . implode(' ' . $subquery['where_joiner'] . ' ', $subquery['where']);
		}

		$db_type = DAL::getDbType();

		/**
		 * If/only if we are checking permissions,
		 * make the permission checking a subquery off the other sub query.
		 * No need to include this if we're not checking permissions.
		*/
		if (!empty($permission_check_query['where'])) {
			/**
			 * Add an extra condition here.
			 * We can push the assetid check (retrieved in the outer query)
			 * into the subquery as well
			 * which speeds up permission checking
			 */
			$permission_check_query['where'][] = 'a.assetid=p.assetid';

			/**
			 * put the base permission check query together
			 *
			 * we use it as a subquery:
			 * .. a.assetid IN ( permission_check_query )
			 */
			$permission_check_query_string = 'SELECT ' . implode(',', $permission_check_query['select']) . ' FROM ' . implode(',', $permission_check_query['from']) . ' ' . implode(' ', $permission_check_query['join']);
			$permission_check_query_string .= ' WHERE ' . implode(' ' . $permission_check_query['where_joiner'] . ' ', $permission_check_query['where']);
			$permission_check_query_string .= ' GROUP BY ' . implode(',', $permission_check_query['select']);
			$permission_check_query_string .= ' HAVING MIN(p.granted) <> \'0\'';

			if (!empty($subquery['where'])) {
				$subquery_string .= ' AND ';
			} else {
				$subquery_string .= ' WHERE ';
			}
			$subquery_string .= ' a.assetid IN (';
			$subquery_string .= $permission_check_query_string;
			$subquery_string .= ')';
		}

		/**
		 * If we're using oracle, then do an IN() query
		 * We can't use the join trick below because it gives an error
		 * about "inconsistent datatypes"
		 */
		if ($db_type == 'oci')  {
			$query['where'][] = 'ai.assetid IN (' . $subquery_string . ')';
		}

		/**
		 * If we're using postgres, do it as a JOIN instead
		 * it gives much better performance
		 */
		if ($db_type == 'pgsql')  {
			$subquery_string = '(' . $subquery_string . ') asset_check';
			$query['from'][] = $subquery_string;
			$query['where'][] = 'ai.assetid=asset_check.assetid';
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
			$db_type = DAL::getDbType();

			$dir = dirname(__FILE__).'/plugins/'.strtolower($db_type);
			$class_name = 'search_manager_plugin_'.$db_type;
			require_once $dir.'/'.$class_name.'.inc';
			$this->_db_plugin = new $class_name();
		}

		return $this->_db_plugin;

	}//end _getDBPlugin()


	/**
	* Reduce results to be keyed based on asset ID
	*
	* @param array	$results	raw results array
	*
	* @return array
	* @access public
	*/
	function getCondensedResults($results)
	{
		$searched_results = Array();

		// transform the array into a better working format
		foreach ($results as $result) {
			if (empty($searched_results[$result['assetid']])) {
					$searched_results[$result['assetid']]['score'] = 0;
			}
			$searched_results[$result['assetid']]['sources'][] = $result['source'];
			$searched_results[$result['assetid']]['score'] += $result['search_score'];
		}

		uasort($searched_results, create_function('$a,$b', 'return $b["score"] - $a["score"];'));

		return $searched_results;

	}//end getCondensedResults()


	/**
	* Provide generic format of search results data
	*
	* Results array is in the format returned by search manager
	* when a detailed searched is made
	*
	* @param array	$results		*CONDENSED* search results (returned by
	*								getCondensedResults())
	* @param string	$search_string	string searched for
	*
	* @param string	$full_content	return full formatted content or not
	*
	* @return array
	* @access public
	*/
	function getFormattedResults($results, $search_string, $full_content = TRUE)
	{
		$search_string = preg_replace('/([\.\\\!\+\*\?\[\]\^\$\(\)\=\!\<\>\|\:]+)/i', '\\\${1}',$search_string);
		$search_string = trim(preg_replace('/[\~\&\|\/]/i', ' ', $search_string));

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$plugin =& $this->_getDBPlugin();

		$formatted_results = Array();

		$words = $plugin->getWords($search_string);

		// populate the results array as required
		// note we dont include fields that dont have matches
		foreach ($results as $assetid => $content) {
			$content = $content['sources'];
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			foreach ($content as $content_type) {
				switch (TRUE) {

					// asset contents containing search string
					case preg_match('/contents/', $content_type) > 0:
						// not need to retrieve full content which could be time wasting
						 if(!$full_content) {
							$formatted_results[$assetid]['contents'][0]='...'.$search_string.'...';
							break;
						 }
						$asset_contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($assetid);

						// If there is no editable contents for the asset, see if the content was generated (eg; for PDF files)
						if (!$asset_contents) {
							$asset_contents = Array(
												$assetid	=> $asset->getContent(),
											  );
						}

						foreach ($asset_contents as $contents_id => $contents_value) {

							foreach ($words as $word) {
								if (preg_match('/'.htmlentities($word).'/i', $contents_value) || preg_match('/'.($word).'/i', $contents_value)) {
									$formatted_results[$assetid]['contents'][$contents_id] = $contents_value;
									break;
								}
							}
						}
					break;

					// attributes containing search string
					// attributes come in 2 formats, __attribute__ and attr:attrname
					case preg_match('/__(.+)__/', $content_type, $matches) > 0:
					case preg_match('/attr:(.+)$/', $content_type, $matches) > 0:
						if (isset($asset->vars[$matches[1]])) {
							$attribute = $asset->attr($matches[1]);

							foreach ($words as $word) {
								if (preg_match("/$word/i", $attribute)) {
									$formatted_results[$assetid]['attributes'][$matches[1]] = $attribute;
									break;
								}
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
							foreach ($words as $word) {
								if (preg_match("/$word/i", $data[0]['value'])) {
									$formatted_results[$assetid]['metadata'][$fieldid] = $data;
									break;
								}
							}
						}

						// if no metadata was set, then we were looking for the schema
						if (!isset($formatted_results[$assetid]['metadata'])) {
							$schemas = $mm->getSchemas($assetid, TRUE);

							foreach ($schemas as  $schemaid) {
								$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
								$schema_data = $mm->getSchemaDefaultValues($schema->id);
								foreach ($schema_data as $fieldid => $data) {
									foreach ($words as $word) {
										if (preg_match("/$word/i", $data['value'])) {
											$formatted_results[$schema->id]['schema'][$fieldid] = $data;
											break;
										}
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
