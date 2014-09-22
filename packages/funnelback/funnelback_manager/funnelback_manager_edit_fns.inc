<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: funnelback_manager_edit_fns.inc,v 1.17 2013/08/06 05:21:01 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Funnelback_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Returns true if funnelback indexing is on.
	*
	* @param object	&$owner	the search manager
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function indexingOn(&$owner, $prefix)
	{
		return $owner->attr('indexing');

	}//end indexingOn()


	/**
	* Paints the interface for enabling indexing by Funnelback
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintIndexing(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$indexing = $asset->attr('indexing');
		$options = Array('0'=>translate('Disabled'),'1'=>translate('Enabled'));


		if ($wa) {
			combo_box($prefix.'_indexing', $options, FALSE, (($indexing) ? '1' : '0'));
		} else {
			echo (($indexing) ? translate('Enabled') : translate('Disabled'));


		}//end if

		return TRUE;

	}//end paintIndexing()


	/**
	* Processes the interface for enabling indexing by Funnelback
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processIndexing(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		$choice = array_get_index($_POST, $prefix.'_indexing', FALSE);
		if ($wa) {
			if ($choice == '1') {
				$asset->setAttrValue('indexing', TRUE);

				if ($asset->checkInstalled() === FALSE) {
					log_dump('Funnelback is enabled, but the binaries are not installed');
				}
			} else {
				$asset->setAttrValue('indexing', FALSE);
			}//end if

			$status = $asset->saveAttributes();
		}//end if

		return $status;

	}//end processIndexing()


	/**
	* Paints the interface for rebuilding the cache
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintRebuildCache(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			asset_finder($prefix.'_rebuild_root_node', 0);

			$o->closeField();
			$o->openField(translate('System Wide'));


			check_box($prefix.'_rebuild_all_system', 1);
			echo translate('Rebuild the cache for the whole system');

		}//end if

		return TRUE;

	}//end paintRebuildCache()


	/**
	* Processes the interface for rebuilding the cache
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processRebuildCache(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$node_info  = array_get_index($_POST, $prefix.'_rebuild_root_node', Array());
			$all_system = array_get_index($_POST, $prefix.'_rebuild_all_system', 0);
			$root_node  = array_get_index($node_info, 'assetid', 0);

			if (!empty($all_system)) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$running_vars = Array('root_assetid' => Array());
				$hh->queueHipo('hipo_job_funnelback_rebuild_cache', $running_vars, '', SQ_PACKAGES_PATH.'/funnelback/hipo_jobs');
			} else if (!empty($root_node) && $GLOBALS['SQ_SYSTEM']->am->assetExists($root_node)) {
				// Indexing by a root node
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$running_vars = Array('root_assetid' => Array($root_node));
				$hh->queueHipo('hipo_job_funnelback_rebuild_cache', $running_vars, '', SQ_PACKAGES_PATH.'/funnelback/hipo_jobs');
			}//end if

			return TRUE;
		}//end if

		return $status;

	}//end processRebuildCache()


// --- INDEXER SCREEN FUNCTIONS --- //


	/**
	* Paints the interface for the Funnelback indexer
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintReindexing(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			if ($this->indexingOn($asset, $prefix)) {
				$collections = $asset->getCollections();
				ksort($collections);
				foreach ($collections as $collection_id => $collection_data) {
					$name = array_get_index($collection_data, 'name', '');
					$valid = array_get_index($collection_data, 'valid', TRUE);
					if (empty($name)) continue;
					check_box($prefix.'_reindex_collection[]', $collection_id);
					echo $name.'<br />';
				}//end foreach

				$o->closeField();
				$o->openField(translate('System Wide'));


				check_box($prefix.'_reindex_all_system', 1);
				echo translate('Index the whole system');

			} else {
				echo translate('Indexing is not enabled');

			}//end if
		}//end if

		return TRUE;

	}//end paintReindexing()


	/**
	* Processes the interface for the Funnelback Indexer
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processReindexing(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$collections = array_get_index($_POST, $prefix.'_reindex_collection', Array());
			$all_system  = array_get_index($_POST, $prefix.'_reindex_all_system', 0);
			$reindex     = FALSE;

			if (!empty($all_system)) {
				// Indexing the whole system
				$collections = Array();
				$reindex = TRUE;
			} else if (!empty($collections)) {
				// Indexing by a root node
				$reindex = TRUE;
			}//end if

			if ($reindex) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$running_vars = Array('collections' => $collections);
				$hh->queueHipo('hipo_job_funnelback_reindex', $running_vars, '', SQ_PACKAGES_PATH.'/funnelback/hipo_jobs');
				return TRUE;
			}//end if
		}//end if

		return FALSE;

	}//end processReindexing()


// --- COLLECTION SCREEN FUNCTIONS --- //


	/**
	* Paints the interface for collections
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintNewCollection(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		if ($wa) {
			// New collection
			$js = 'onChange="javascript:funnelbackDisableURL(this);"';
			$o->openField(translate('Collection ID'), '', translate('This is an internal identifier that will be used to uniquely name your collection. It cannot contain spaces.'));


				text_box($prefix.'_new_collection');
			$o->closeField();
			$o->openField(translate('Collection Name'), '', translate('This is the title for the collection that will appear in the search results.'));


				text_box($prefix.'_new_collection_name');
			$o->closeField();
			$o->openField(translate('Collection Type'));

				combo_box($prefix.'_new_collection_type', $asset->_getCollectionTypes(), FALSE, 'web', 0, $js);
			$o->closeField();
		}//end if

		return TRUE;

	}//end paintNewCollection()


	/**
	* Processes the interface for collections
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processNewCollection(&$asset, &$o, $prefix)
	{
		$saved = FALSE;
		$wa = $asset->writeAccess('attributes');

		if ($wa) {
			$new_collection = array_get_index($_POST, $prefix.'_new_collection', '');
			$new_collection_name = array_get_index($_POST, $prefix.'_new_collection_name', '');
			$new_collection_type = array_get_index($_POST, $prefix.'_new_collection_type', 'web');
			$collections = $asset->getCollections();
			$new_collection = str_replace(' ', '_', $new_collection);
			if (!empty($new_collection) && !isset($collections[$new_collection])) {
				$collections[$new_collection] = Array(
													'name'	=> $new_collection_name,
													'type'	=> $new_collection_type,
													'asset'	=> 0,
												);
				$asset->setAttrValue('collections', $collections);
				$saved = $asset->saveAttributes();
				$coll_path = $asset->getCollectionDataPath($new_collection);
				$config_files = $asset->getCollectionConfigFiles($new_collection);
				foreach ($config_files as $type => $config) {
					$asset->createCollectionConfig($config, $type);
				}//end foreach
			}//end if
		}//end if

		return $saved;

	}//end processNewCollection()


	/**
	* Paints the interface for collections
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintCollections(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_id = array_get_index($_POST, $prefix.'_edit_collection', '');
		hidden_field($prefix.'_edit_collection', $collection_id);
		?>
		<table class="sq-backend-table" style="width:100%">
			<tr>
			<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('Edit'); ?></td><?php } ?>

			<td class="sq-backend-table-header" width="20%"><?php echo translate('Collection ID'); ?></td>

			<td class="sq-backend-table-header" width="50%"><?php echo translate('Collection Name'); ?></td>

			<td class="sq-backend-table-header" width="20%"><?php echo translate('Collection Type'); ?></td>

			<td class="sq-backend-table-header" width="20%"><?php echo translate('Valid?'); ?></td>

			<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('Delete'); ?>?</td><?php } ?>

			</tr>
		<?php
		if (!empty($collections)) {
			foreach ($collections as $coll_id => $collection_info) {
				if ($collection_id == $coll_id) {
					$class = 'sq-table-row alt';
				} else {
					$class = 'sq-table-row';
				}//end if
				echo '<tr class="'.$class.'"><td>';
				if ($wa) {
					$img = '<a href="#" onclick="document.getElementById(\''.$prefix.'_edit_collection\').value = \''.$coll_id.'\'; document.main_form.submit(); return false;"><img src="'.sq_web_path('lib').'/web/images/icons/edit_mode.png" width="15" height="15" border="0" /></a>';
					echo '<p>'.$img.'</p>';
					echo '</td><td>';
				}//end if
				echo $coll_id;
				echo '</td><td>';
				echo array_get_index($collection_info, 'name', '');
				echo '</td><td>';
				$current_type = array_get_index($collection_info, 'type', 'web');
				$types = $asset->_getCollectionTypes();
				echo array_get_index($types, $current_type, $types['web']);
				echo '</td><td>';
				if (array_get_index($collection_info, 'valid', FALSE) && $this->_checkCollection($collections, $coll_id)) {
					$icon = 'tick';
				} else {
					$icon = 'cross';
				}//end if
				echo '<img src="'.sq_web_path('lib').'/web/images/'.$icon.'.gif" border="0" />';
				if ($wa) {
					echo '</td><td>';
					check_box($prefix.'_delete[]', $coll_id);
				}//end if
				echo '</td></tr>';
			}//end foreach
		} else {
			$cols = ($wa) ? '6' : '4';
			echo '<tr><td align="center" colspan="'.$cols.'">'.translate('No Collections Set').'</td></tr>';

		}//end if
		?>
		</table>
		<?php

		return TRUE;

	}//end paintCollections()


	/**
	* Processes the interface for collections
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processCollections(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$delete_collection = array_get_index($_POST, $prefix.'_delete', Array());
		if ($wa) {
			if (!empty($delete_collection)) {
				foreach($delete_collection as $del_collection) {
					if (isset($collections[$del_collection])) {
						// Get the collection data path
						$coll_path = $asset->getCollectionDataPath($del_collection);
						if (is_dir($coll_path)) {
							require_once SQ_FUDGE_PATH.'/general/file_system.inc';
							delete_directory($coll_path);
						}//end if

						// 'Delete' the collection
						unset($collections[$del_collection]);
					}//end if
				}//end foreach
				$asset->setAttrValue('collections', $collections);
				$status = $asset->saveAttributes();
			}//end if
		}//end if

		return $status;

	}//end processCollections()


	/**
	* Paints the interface for editing a selected collection
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintSelectedCollection(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_id = array_get_index($_POST, $prefix.'_edit_collection', NULL);

		if ($wa) {
			if (!empty($collection_id) && isset($collections[$collection_id])) {
				$current_type = array_get_index($collections[$collection_id], 'type', 'web');
				$current_name = array_get_index($collections[$collection_id], 'name', '');
				$current_assetid = array_get_index($collections[$collection_id], 'asset', Array());
				$current_url = array_get_index($collections[$collection_id], 'url', Array());
				$o->openField(translate('Collection Name'));

					hidden_field($prefix.'_edit_collection_id', $collection_id);
					text_box($prefix.'_edit_collection_name', $current_name);
				$o->closeField();
				switch ($current_type) {
					case 'meta':
						$o->openField(translate('Collections'));

							$all_collections = $asset->getCollections();
							$selected_collections = array_get_index($collections[$collection_id], 'collections', Array());
							unset($all_collections[$collection_id]);
							?>
							<table class="sq-backend-table" style="width:70%">
								<tr>
								<td class="sq-backend-table-header" width="80%">
									<?php echo translate('Collection Name'); ?>

								</td>
								<td class="sq-backend-table-header"><?php echo translate('Please Select'); ?>?</td>

								</tr>
							<?php
							foreach ($all_collections as $each_collection_id => $each_collection) {
								$selected = (in_array($each_collection_id, $selected_collections)) ? TRUE : FALSE;
								$name = array_get_index($each_collection, 'name', '');
								if (empty($name)) continue;
								echo '<tr><td>';
								echo $name;
								echo '</td><td>';
								check_box($prefix.'_edit_collection_meta_collections[]', $each_collection_id, $selected);
								echo '</td></tr>';
							}//end foreach
							?>
							</table>
							<?php
						$o->closeField();
					break;

					default:
						$o->openField(translate('Collection Source Asset'));

							asset_finder($prefix.'_edit_collection_source', $current_assetid);
						$o->closeField();
						$o->openField(translate('Collection URL'));

							$urls = Array();
							if (!empty($current_assetid)) {
								$current_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_assetid, '', TRUE);
								if (!is_null($current_asset)) {
									$possible_urls = $current_asset->getURLs();
									foreach ($possible_urls as $possible_url) {
										$http  = array_get_index($possible_url, 'http', FALSE);
										$https = array_get_index($possible_url, 'https', FALSE);
										$purl  = array_get_index($possible_url, 'url', '');

										if ($http && !empty($purl)) {
											$urls[] = 'http://'.$purl;
										}
										if ($https && !empty($purl)) {
											$urls[] = 'https://'.$purl;
										}
									}//end foreach
								}//end if
							}//end if

							if (empty($urls)) {
								echo translate('No URL found');

							} else {
								foreach ($urls as $url) {
									check_box($prefix.'_edit_collection_url[]', $url, ((in_array($url, $current_url)) ? TRUE : FALSE));
									echo ' '.htmlspecialchars($url).'<br />';
								}//end foreach
							}//end if
						$o->closeField();
				}//end switch
			}//end if
		}//end if

		return TRUE;

	}//end paintSelectedCollection()


	/**
	* Processes the interface for editing a selected collection
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processSelectedCollection(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$changed = FALSE;
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_id = array_get_index($_POST, $prefix.'_edit_collection_id', NULL);

		if ($wa) {
			if (!empty($collection_id) && isset($collections[$collection_id])) {
				$name = array_get_index($_POST, $prefix.'_edit_collection_name', '');
				$source = array_get_index($_POST, $prefix.'_edit_collection_source', '');
				$url = array_get_index($_POST, $prefix.'_edit_collection_url', Array());
				$current_type = array_get_index($collections[$collection_id], 'type', 'web');
				if ($current_type == 'meta') {
					$meta_collections = array_get_index($_POST, $prefix.'_edit_collection_meta_collections', Array());
					$collections[$collection_id]['valid'] = FALSE;
					$collections[$collection_id]['collections'] = $meta_collections;
					// Create the meta collection
					$index_paths = Array();
					foreach ($meta_collections as $meta_collection) {
						$meta_collection_data_path = $asset->getCollectionDataPath($meta_collection, 'idx');
						if (!empty($meta_collection_data_path) && is_dir($meta_collection_data_path)) {
							$index_paths[] = $meta_collection_data_path.'/'.$meta_collection;
						}//end if
					}//end foreach
					$data_path = $asset->getCollectionDataPath($collection_id, 'idx');
					// Create the meta collection
					// No indexing is needed as the other collections it uses, should handle indexing themselves
					$create_status = file_put_contents($data_path.'/'.$collection_id.'.sdinfo', implode("\n", $index_paths));
					if ($create_status !== FALSE && !empty($index_paths)) {
						$collections[$collection_id]['valid'] = TRUE;
					}//end if
					$changed = TRUE;
				}//end if
				if (!empty($name)) {
					$collections[$collection_id]['name'] = $name;
					$changed = TRUE;
				}//end if
				if (!empty($source)) {
					$source_assetid = array_get_index($source, 'assetid', 0);
					if (!empty($source_assetid)) {
						$source_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($source_assetid);
						if (!empty($source_asset)) {
							$asset_url = $source_asset->getURL();
							$unrestricted_asset = strpos($asset_url, '/__data/') !== FALSE ||
												(SQ_CONF_STATIC_ROOT_URL && strpos($asset_url, SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($source_asset);

							if ($unrestricted_asset) {
								trigger_localised_error('FNB0015', translate('Cannot use an unrestrict asset as a collection'), E_USER_WARNING)
								return FALSE;
							}
							$collections[$collection_id]['asset'] = $source_assetid;
							$changed = TRUE;
						}//end if
					}//end if
				}//end if
				if ($current_type == 'web') {
					$collections[$collection_id]['url'] = $url;
					if (empty($url) || empty($collections[$collection_id]['asset'])) {
						$collections[$collection_id]['valid'] = FALSE;
					}//end if
					$changed = TRUE;
				}//end if

				if ($changed) {
					// First check, and make valid if an assetid/url are set correctly
					$collections = $this->_checkCollection($collections);
					$asset->setAttrValue('collections', $collections);
					$status = $asset->saveAttributes();
				}//end if
			}//end if
		}//end if

		return $status;

	}//end processSelectedCollection()


// --- THESAURUS SCREEN FUNCTIONS --- //


	/**
	* Paints the interface for choosing a thesaurus to edit
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintThesaurusChooser(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_selecter = Array();
		$current_choice = array_get_index($_POST, $prefix.'_choose_thesaurus', '');
		foreach ($collections as $id => $collection) {
			$collection_selecter[$id] = array_get_index($collection, 'name', '');
		}//end foreach
		ksort($collection_selecter);

		if ($wa) {
			if (!empty($collection_selecter)) {
				echo combo_box($prefix.'_choose_thesaurus', $collection_selecter, FALSE, $current_choice);
			} else {
				echo translate('No Collections have been set, please go to the \'Collections\' screen and create a collection before continuing');

			}//end if
		}//end if

		return TRUE;

	}//end paintThesaurusChooser()


	/**
	* Processes the interface for choosing a thesaurus to edit
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processThesaurusChooser(&$asset, &$o, $prefix)
	{
		// I don't do anything ;)

		return TRUE;

	}//end processThesaurusChooser()


	/**
	* Paints the interface for editing a thesaurus
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintThesaurus(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$current_choice = array_get_index($_POST, $prefix.'_choose_thesaurus', '');
		hidden_field($prefix.'_chosen_thesaurus', $current_choice);
		$thesaurus_types = Array('term'=>translate('Term by Term Match'), 'whole'=>translate('Whole Query Match'), 'regex'=>translate('Regular Expression Match'));



		if ($wa && !empty($current_choice) && isset($collections[$current_choice])) {
			$thesaurus_config = $asset->getCollectionDataPath($current_choice, 'conf').'/synonyms.cfg';
			$config = $asset->readConfiguration($thesaurus_config);
			?>
			<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('Query Type'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Term'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Replacement'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Delete'); ?></td>

			</tr>
			<?php
			// Current thesaurus
			$count = 1;
			foreach ($config as $line) {
				echo '<tr><td>';
				echo combo_box($prefix.'_thesaurus_query['.$count.']', $thesaurus_types, FALSE, $line['query']);
				echo '</td><td>';
				echo text_box($prefix.'_thesaurus_term['.$count.']', $line['term'], 30);
				echo '</td><td>';
				echo text_box($prefix.'_thesaurus_expansion['.$count.']', $line['expansion'], 30);
				echo '</td><td>';
				echo check_box($prefix.'_thesaurus_delete['.$count.']', '1');
				echo '</td></tr>';
				$count++;
			}//end foreach

			// Add new ones
			for ($i=$count; $i<=($count+1); $i++) {
				echo '<tr><td>';
				echo combo_box($prefix.'_thesaurus_query['.$i.']', $thesaurus_types);
				echo '</td><td>';
				echo text_box($prefix.'_thesaurus_term['.$i.']', '', 30);
				echo '</td><td>';
				echo text_box($prefix.'_thesaurus_expansion['.$i.']', '', 30);
				echo '</td><td>';
				echo check_box($prefix.'_thesaurus_delete['.$i.']', '1');
				echo '</td></tr>';
			}//end for
			?>
			</table>
			<?php
		} else if (!empty($current_choice) && !isset($collections[$current_choice])) {
			echo sprintf(translate('Missing Collection: %s'), $current_choice);

		}//end if

		return TRUE;

	}//end paintThesaurus()


	/**
	* Processes the interface for editing a thesaurus
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processThesaurus(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		if (!$wa) return $status;

		$collection = array_get_index($_POST, $prefix.'_chosen_thesaurus', '');
		if (empty($collection)) return $status;

		// Grab all the terms, expansions etc.
		$query = array_get_index($_POST, $prefix.'_thesaurus_query', Array());
		$term = array_get_index($_POST, $prefix.'_thesaurus_term', Array());
		$expansion = array_get_index($_POST, $prefix.'_thesaurus_expansion', Array());
		$delete = array_get_index($_POST, $prefix.'_thesaurus_delete', Array());

		if (empty($query) || (count($query) != count($term) && count($query) != count($expansion))) {
			return $status;
		}//end if

		$data = Array();
		foreach ($query as $id => $each_query) {
			if (!isset($delete[$id]) || $delete[$id] != '1') {
				$query_type = '+';
				if ($each_query == 'term') {
					$query_type = '%';
				} else if ($each_query == 'regex') {
					$query_type = '~';
				}//end if
				$data[] = Array('query'=>$query_type, 'term'=>$term[$id], 'expansion'=>$expansion[$id]);
			}//end if
		}//end foreach

		$config = $asset->getCollectionDataPath($collection, 'conf').'/synonyms.cfg';
		$asset->writeConfiguration($config, $data);

		return $status;

	}//end processThesaurus()


// --- FEATURE PAGES SCREEN FUNCTIONS --- //


	/**
	* Paints the interface for choosing a feature page to edit
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintFeaturePageChooser(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_selecter = Array();
		$current_choice = array_get_index($_POST, $prefix.'_edit_feature_page', '');
		foreach ($collections as $id => $collection) {
			$collection_selecter[$id] = array_get_index($collection, 'name', '');
		}//end foreach
		ksort($collection_selecter);

		if ($wa) {
			if (!empty($collection_selecter)) {
				echo combo_box($prefix.'_edit_feature_page', $collection_selecter, FALSE, $current_choice);
			} else {
				echo translate('No Collections have been set, please go to the \'Collections\' screen and create a collection before continuing');

			}//end if
		}//end if

		return TRUE;

	}//end paintFeaturePageChooser()


	/**
	* Processes the interface for adding a new feature page for a collection
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processFeaturePageChooser(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collection_id = array_get_index($_POST, $prefix.'_edit_feature_page', '');
		if ($wa) {
			$current_feature_pages = $asset->attr('feature_pages');
			if (!empty($collection_id) && !isset($current_feature_pages[$collection_id])) {
				$current_feature_pages[$collection_id] = Array(
															Array(
																'query'		=> 'substring',
																'trigger'	=> '',
																'assetid'	=> 0,
																'url'		=> '',
																'valid'		=> FALSE,
															),
														 );
				$asset->setAttrValue('feature_pages', $current_feature_pages);
				return $asset->saveAttributes();
			}//end if
		}//end if

		return TRUE;

	}//end processFeaturePageChooser()


	/**
	* Paints the interface for editing a feature page
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintFeaturePage(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$config = $asset->attr('feature_pages');
		$collection_id = array_get_index($_POST, $prefix.'_edit_feature_page', '');
		$line_id = array_get_index($_POST, $prefix.'_edit_feature_page_line', '');
		hidden_field($prefix.'_edit_feature_page_collection_id', $collection_id);
		hidden_field($prefix.'_edit_feature_page_line', $line_id);
		if (!empty($collection_id)) {
			$collection_name = array_get_index(array_get_index($collections, $collection_id, Array()), 'name', '');
			echo sprintf(translate('<b>Editing Collection: %s</b>'), $collection_name);

			?>
			<table class="sq-backend-table">
			<tr>
				<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('Edit'); ?></td><?php } ?>

				<td class="sq-backend-table-header"><?php echo translate('Trigger Word'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Valid?'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('URL'); ?></td>

				<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('Delete'); ?>?</td><?php } ?>

			</tr>
			<?php
			if (isset($config[$collection_id])) {
				// Current feature pages
				foreach ($config[$collection_id] as $index => $feature_page) {
					$feature_trigger = array_get_index($feature_page, 'trigger', '');
					$feature_url = array_get_index($feature_page, 'url', '');
					if ($index == $line_id) {
						$class = 'sq-table-row alt';
					} else {
						$class = 'sq-table-row';
					}//end if
					echo '<tr class="'.$class.'"><td>';
					if ($wa) {
						$img = '<a href="#" onclick="document.getElementById(\''.$prefix.'_edit_feature_page_line\').value = \''.$index.'\'; document.getElementById(\''.$prefix.'_edit_selected_feature_page_changed\').value = \'0\'; document.main_form.submit(); return false;"><img src="'.sq_web_path('lib').'/web/images/icons/edit_mode.png" width="15" height="15" border="0" /></a>';
						echo '<p>'.$img.'</p>';
						echo '</td><td>';
					}//end if
					echo $feature_trigger;
					echo '</td><td>';
					if (array_get_index($feature_page, 'valid', FALSE)) {
						$icon = 'tick';
					} else {
						$icon = 'cross';
					}//end if
					echo '<img src="'.sq_web_path('lib').'/web/images/'.$icon.'.gif" border="0" />';
					echo '</td><td width="50%">';
					echo htmlspecialchars($feature_url, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					echo '</td><td>';
					echo check_box($prefix.'_feature_delete['.$index.']', '1');
					echo '</td></tr>';
				}//end foreach
			}//end if
			?>
			</table>
			<?php

			if ($wa) {
				echo int_text_box($prefix.'_add_new', 0, FALSE, 5, 0, 10);
				echo ' '.translate('Add new feature page entries');

			}//end if
		} else {
			echo translate('Choose a collection to start editing');

		}//end if

		return TRUE;

	}//end paintFeaturePage()


	/**
	* Processes the interface for editing a feature page
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processFeaturePage(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$collection_id = array_get_index($_POST, $prefix.'_edit_feature_page_collection_id', '');
			$feature_pages = $asset->attr('feature_pages');
			if (!empty($collection_id) && isset($feature_pages[$collection_id])) {
				$feature_delete = array_get_index($_POST, $prefix.'_feature_delete', Array());
				$new_feature = Array();
				$changed = FALSE;
				foreach ($feature_pages[$collection_id] as $index => $current_settings) {
					if (isset($feature_delete[$index]) && $feature_delete[$index] == '1') {
						unset($feature_pages[$collection_id][$index]);
						$changed = TRUE;
					} else {
						$new_feature[] = $current_settings;
					}//end if
				}//end foreach

				$add_new = array_get_index($_POST, $prefix.'_add_new', 0);
				if (!empty($add_new)) {
					$changed = TRUE;
					for ($i=0; $i<$add_new; $i++) {
						$new_feature[] = Array('query'=>'substring','trigger'=>'','url'=>'','assetid'=>0,'valid'=>FALSE);
					}//end for
				}//end if

				if ($changed) {
					$feature_pages[$collection_id] = $new_feature;
					$asset->setAttrValue('feature_pages', $feature_pages);
					$status = $asset->saveAttributes();
					$this->_writeFeaturePagesConfiguration($asset, $collection_id, $new_feature);
				}//end if
			}//end if
		}//end if

		return $status;

	}//end processFeaturePage()


	/**
	* Paints the interface for editing a selected feature page
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintSelectedFeaturePage(&$asset, &$o, $prefix)
	{
		$changed = '0';
		$wa = $asset->writeAccess('attributes');
		$feature_queries = Array(
							'term'		=> translate('Term by Term Match'),

							'exact'		=> translate('Exact Match'),

							'regex'		=> translate('Regular Expression Match'),

							'substring'	=> translate('Substring Match'),

						   );
		if ($wa) {
			$config = $asset->attr('feature_pages');
			$collection_id = array_get_index($_POST, $prefix.'_edit_feature_page_collection_id', '');
			$line_id = array_get_index($_POST, $prefix.'_edit_feature_page_line', '');
			if ($collection_id != '' && $line_id != '') {
				if (isset($config[$collection_id][$line_id])) {
					$changed = '1';
					$line_data = $config[$collection_id][$line_id];
					$o->closeField();
					$o->openField(translate('Query Type'));

						$query = array_get_index($line_data, 'query', 'substring');
						echo combo_box($prefix.'_edit_selected_feature_page_query', $feature_queries, FALSE, $query);
					$o->closeField();
					$o->openField(translate('Trigger Word'));

						$trigger = array_get_index($line_data, 'trigger', '');
						echo text_box($prefix.'_edit_selected_feature_page_trigger', $trigger, 25);
					$o->closeField();
					$o->openField(translate('Page Title'));

						$title = array_get_index($line_data, 'title', '');
						echo text_box($prefix.'_edit_selected_feature_page_title', $title, 25);
					$o->closeField();
					$o->openField(translate('Page Description'));

						$desc = array_get_index($line_data, 'desc', '');
						echo text_area($prefix.'_edit_selected_feature_page_desc', $desc, 22, 3);
					$o->closeField();
					$o->openField(translate('Asset'));

						$assetid = array_get_index($line_data, 'assetid', 0);
						echo asset_finder($prefix.'_edit_selected_feature_page_assetid', $assetid, Array('page'=>'D','site'=>'D'));
					$o->closeField();
					$o->openField(translate('URL'));

						$url = array_get_index($line_data, 'url', '');
						if ($GLOBALS['SQ_SYSTEM']->am->assetExists($assetid)) {
							$asset_urls = Array();
							$raw_urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($assetid);
							foreach ($raw_urls as $raw_url) {
								$base_url = array_get_index($raw_url, 'url', '');
								$http = array_get_index($raw_url, 'http', 0);
								$https = array_get_index($raw_url, 'https', 0);
								if ($http) {
									$new_url = 'http://'.$base_url;
									$asset_urls[$new_url] = $new_url;
								}//end if
								if ($https) {
									$new_url = 'https://'.$base_url;
									$asset_urls[$new_url] = $new_url;
								}//end if
							}//end foreach
							echo combo_box($prefix.'_edit_selected_feature_page_url', $asset_urls, FALSE, $url);
						}//end if
				}//end if
			}//end if
		}//end if
		hidden_field($prefix.'_edit_selected_feature_page_changed', $changed);

		return TRUE;

	}//end paintSelectedFeaturePage()


	/**
	* Processes the interface for editing a feature page
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processSelectedFeaturePage(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$config = $asset->attr('feature_pages');
			$collection_id = array_get_index($_POST, $prefix.'_edit_feature_page', '');
			$line_id = array_get_index($_POST, $prefix.'_edit_feature_page_line', '');
			if ($collection_id != '' && $line_id != '') {
				if (isset($config[$collection_id][$line_id])) {
					$changed = array_get_index($_POST, $prefix.'_edit_selected_feature_page_changed', '0');
					$query = array_get_index($_POST, $prefix.'_edit_selected_feature_page_query', 'substring');
					$trigger = array_get_index($_POST, $prefix.'_edit_selected_feature_page_trigger', '');
					$title = array_get_index($_POST, $prefix.'_edit_selected_feature_page_title', '');
					$desc = array_get_index($_POST, $prefix.'_edit_selected_feature_page_desc', '');
					$asset_info = array_get_index($_POST, $prefix.'_edit_selected_feature_page_assetid', Array());
					$assetid = array_get_index($asset_info, 'assetid', 0);
					if (!empty($assetid)) {
						$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetid, Array(), TRUE, 'type_code');
						if (isset($asset_info[$assetid])) {
							$asset_type = $asset_info[$assetid];
							if (!($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($asset_type, 'page') || $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($asset_type, 'site'))) {
								$assetid = 0;
							}//end if
						} else {
							$assetid = 0;
						}//end if
					}//end if
					$url = array_get_index($_POST, $prefix.'_edit_selected_feature_page_url', '');
					$edit_feature = Array('query'=>$query,'trigger'=>$trigger,'assetid'=>$assetid,'url'=>$url,'valid'=>FALSE);
					if (!empty($title)) $edit_feature['title'] = $title;
					if (!empty($desc)) $edit_feature['desc'] = $desc;

					// Check the feature page is valid
					if ($GLOBALS['SQ_SYSTEM']->am->assetExists($assetid)) {
						$asset_urls = Array();
						$raw_urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($assetid);
						foreach ($raw_urls as $raw_url) {
							$base_url = array_get_index($raw_url, 'url', '');
							$http = array_get_index($raw_url, 'http', 0);
							$https = array_get_index($raw_url, 'https', 0);
							if ($http) {
								$new_url = 'http://'.$base_url;
								$asset_urls[$new_url] = $new_url;
							}//end if
							if ($https) {
								$new_url = 'https://'.$base_url;
								$asset_urls[$new_url] = $new_url;
							}//end if
						}//end foreach
						if (in_array($url, $asset_urls)) {
							$edit_feature['valid'] = TRUE;
						} else {
							$edit_feature['url'] = '';
						}//end if
					}//end if

					if ($changed == '1') {
						$config[$collection_id][$line_id] = $edit_feature;
						$asset->setAttrValue('feature_pages', $config);
						$status = $asset->saveAttributes();
						$this->_writeFeaturePagesConfiguration($asset, $collection_id, $config[$collection_id]);
					}//end if
				}//end if
			}//end if
		}//end if

		return $status;

	}//end processSelectedFeaturePage()


// --- FLUSTER (CONTEXTUAL NAVIGATION) SCREEN FUNCTIONS --- //


	/**
	* Paints the interface for choosing a collection to edit
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintFlusterChooser(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_selecter = Array();
		$current_choice = array_get_index($_POST, $prefix.'_edit_fluster', '');
		foreach ($collections as $id => $collection) {
			$collection_selecter[$id] = array_get_index($collection, 'name', '');
		}//end foreach
		ksort($collection_selecter);

		if ($wa) {
			if (!empty($collection_selecter)) {
				echo combo_box($prefix.'_edit_fluster', $collection_selecter, FALSE, $current_choice);
			} else {
				echo translate('No Collections have been set, please go to the \'Collections\' screen and create a collection before continuing');

			}//end if
		}//end if

		return TRUE;

	}//end paintFlusterChooser()


	/**
	* Processes the interface for choosing a fluster for a collection
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processFlusterChooser(&$asset, &$o, $prefix)
	{
		// I do nothing ;)

		return TRUE;

	}//end processFlusterChooser()


	/**
	* Paints the interface for editing flustering options
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintFluster(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_id = array_get_index($_POST, $prefix.'_edit_fluster', '');
		if ($wa && !empty($collection_id) && isset($collections[$collection_id])) {
			$coll_conf_path = $asset->getCollectionDataPath($collection_id, 'conf');
			$fluster_config = $coll_conf_path.'/contextual_navigation.cfg';
			$asset->createCollectionConfig($fluster_config, 'fluster');
			$config = $asset->readConfiguration($fluster_config, 'fluster');
			$multi_opts = Array('cannot_end_with', 'kill_list');
			$bool_opts = Array('categorise_clusters','enable_fluster');
			hidden_field($prefix.'_fluster_collection_id', $collection_id);
			foreach ($config as $switch => $parameter) {
				$name = str_replace('.', '_', $switch);
				$o->closeField();
				$o->openField(ucwords(str_replace('_', ' ', $name)));

				// Do not enable this flag as this we control ourselves
				if ($switch == 'enable_fluster') continue;

				if (in_array($switch, $multi_opts)) {
					// Multi-options available
					hidden_field($prefix.'_fluster_multi_opts[]', $switch);
					require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';
					$opts = new Asset_Attribute_Option_List();
					$opts->_edit_params['allow_reorder'] = FALSE;
					$opts->delimiter = ',';
					$opts->value = $parameter;
					$opts->paint($prefix.'_fluster_opt_'.$name);
				} else if (in_array($switch, $bool_opts)) {
					// Boolean options
					$options = Array('True' => translate('True'), 'False' => translate('False'));


					if (!array_key_exists($parameter, $options)) $parameter = 'false';
					hidden_field($prefix.'_fluster_opts[]', $switch);
					combo_box($prefix.'_fluster_opt_'.$name, $options, FALSE, $parameter);
				} else {
					hidden_field($prefix.'_fluster_opts[]', $switch);
					text_box($prefix.'_fluster_opt_'.$name, $parameter);
				}//end if
			}//end foreach
		} else {
			echo translate('Choose a collection to start editing');

		}//end if

		return TRUE;

	}//end paintFluster()


	/**
	* Processes the interface for editing flustering options
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processFluster(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$collection_id = array_get_index($_POST, $prefix.'_fluster_collection_id', '');
			if (!empty($collection_id)) {
				$coll_conf_path = $asset->getCollectionDataPath($collection_id, 'conf');
				$fluster_config = $coll_conf_path.'/contextual_navigation.cfg';
				$data = Array();
				$options = array_get_index($_POST, $prefix.'_fluster_opts', Array());
				$multi_options = array_get_index($_POST, $prefix.'_fluster_multi_opts', Array());
				foreach ($options as $opt_name) {
					$name = str_replace('.', '_', $opt_name);
					$data[$opt_name] = array_get_index($_POST, $prefix.'_fluster_opt_'.$name, '');
				}//end foreach
				foreach ($multi_options as $opt_name) {
					$name = str_replace('.', '_', $opt_name);
					require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';
					$opts = new Asset_Attribute_Option_List();
					$opts->_edit_params['allow_reorder'] = FALSE;
					$opts->delimiter = ',';
					$opts->process($prefix.'_fluster_opt_'.$name);
					$data[$opt_name] = $opts->value;
				}//end foreach

				// Saving the file
				if (!empty($data)) {
					$status = $asset->writeConfiguration($fluster_config, $data, 'fluster');
				}//end if
			}//end if
		}//end if

		return $status;

	}//end processFluster()


// --- METADATA SCREEN FUNCTIONS --- //


	/**
	* Paints the interface for choosing a collection to edit
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintMetamapChooser(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_selecter = Array();
		$current_choice = array_get_index($_POST, $prefix.'_edit_metadata', '');
		foreach ($collections as $id => $collection) {
			$collection_selecter[$id] = array_get_index($collection, 'name', '');
		}//end foreach
		ksort($collection_selecter);

		if ($wa) {
			if (!empty($collection_selecter)) {
				echo combo_box($prefix.'_edit_metadata', $collection_selecter, FALSE, $current_choice);
			} else {
				echo translate('No Collections have been set, please go to the \'Collections\' screen and create a collection before continuing');

			}//end if
		}//end if

		return TRUE;

	}//end paintMetamapChooser()


	/**
	* Processes the interface for choosing a collection
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processMetamapChooser(&$asset, &$o, $prefix)
	{
		// I do nothing ;)

		return TRUE;

	}//end processMetamapChooser()


	/**
	* Paints the interface for editing metadata options
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintMetamap(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$collections = $asset->getCollections();
		$collection_id = array_get_index($_POST, $prefix.'_edit_metadata', '');
		if ($wa && !empty($collection_id) && isset($collections[$collection_id])) {
			$coll_conf_path = $asset->getCollectionDataPath($collection_id, 'conf');
			$metadata_config = $coll_conf_path.'/metamap.cfg';
			$metamap_config = $asset->attr('metamap');
			$config = array_get_index($metamap_config, $collection_id, Array());
			if (empty($config)) {
				if (!file_exists($metadata_config)) {
					// Config does not exist create from funnelback install
					$asset->createCollectionConfig($metadata_config, 'metadata');
				}//end if
				$config = $asset->readConfiguration($metadata_config, 'metadata', ',');
			}//end if
			if (!empty($config)) {
				// Set the current collection, so if collection picker changes, we do not save this config to a different collection
				hidden_field($prefix.'_metamap_collection_id', $collection_id);
				?>
				<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header" width="15%"><?php echo translate('Metadata Class'); ?></td>

					<td class="sq-backend-table-header" width="15%"><?php echo translate('Word Logic'); ?></td>

					<td class="sq-backend-table-header" width="15%"><?php echo translate('Flag'); ?></td>

					<td class="sq-backend-table-header" width="60%"><?php echo translate('Tag Name'); ?></td>

					<td class="sq-backend-table-header" width="5%"><?php echo translate('Delete'); ?></td>

				</tr>
				<tr>
					<td class="sq-backend-table-row"><?php echo sprintf(translate('Metadata Class %s'), 'S'); ?></td>

					<td class="sq-backend-table-row" colspan="5"><?php echo translate('Reserved Metadata Class'); ?></td>

				</tr>
				<?php
				$alias_options = $asset->getMetadataAliases();
				$flag_options = Array(0=>translate('No Index'),1=>translate('Index'));


				$word_logic_options = Array(
										0 => translate('Exclude'),

										1 => translate('Include any'),

										2 => translate('Include all'),

									);

				if (isset($alias_options['S'])) unset($alias_options['S']);

				$alias_word_logic = Array();
				$metadata_class_count = 0;
				foreach ($config as $line) {
					$alias = array_get_index($line, 'alias', '');
					$content = array_get_index($line, 'content', '');
					$metatag = array_get_index($line, 'metatag', '');
					$word_logic = array_get_index($line, 'word_logic', 2);
					$system = array_get_index($line, 'system', FALSE);


					// This alias is reserved for Squiz authtentication keys, and for this reason
					// the keys represented by 'S' will be shown as reserved
					if ($alias == 'S') {
						continue;
					}//end if

					echo '<tr>';
					echo '<td class="sq-table-row">';
					if ($system) {
						echo $alias_options[$alias];
						hidden_field($prefix.'_metamap_alias[]', $alias);
					} else {
						combo_box($prefix.'_metamap_alias[]', $alias_options, FALSE, $alias);
					}//end if
					echo '</td>';
					echo '<td class="sq-table-row">';
					if (!isset($alias_word_logic[$alias])) {
						combo_box($prefix.'_metamap_word_logic[]', $word_logic_options, FALSE, $word_logic);
						$alias_word_logic[$alias] = $word_logic;
					} else {
						echo $word_logic_options[$alias_word_logic[$alias]];
						hidden_field($prefix.'_metamap_word_logic[]', $alias_word_logic[$alias]);
					}
					echo '</td>';
					echo '<td class="sq-table-row">';
					if ($system) {
						echo $flag_options[$content];
						hidden_field($prefix.'_metamap_content[]', $content);
					} else {
						combo_box($prefix.'_metamap_content[]', $flag_options, FALSE, $content);
					}//end if
					echo '</td>';
					echo '<td class="sq-table-row">';
					if ($system) {
						echo $metatag;
						hidden_field($prefix.'_metamap_metatag[]', $metatag);
					} else {
						text_box($prefix.'_metamap_metatag[]', $metatag, 30);
					}//end if
					hidden_field($prefix.'_metamap_system[]', (string) $system);
					echo '</td>';
					echo '<td class="sq-table-row">';
					if ($system) {
						hidden_field($prefix.'_metamap_delete['.$metadata_class_count.']', '0');
					} else {
						check_box($prefix.'_metamap_delete['.$metadata_class_count.']', '1');
					}//end if
					echo '</td>';
					echo '</tr>';

					$metadata_class_count++;
				}//end foreach

				// Add new entries
				for ($i=0; $i<2; $i++) {
					echo '<tr>';
					echo '<td class="sq-table-row">';
					combo_box($prefix.'_metamap_alias[]', $alias_options, FALSE, '');
					echo '</td>';
					echo '<td class="sq-table-row">';
					combo_box($prefix.'_metamap_word_logic[]', $word_logic_options, FALSE, 2);
					echo '</td>';
					echo '<td class="sq-table-row">';
					combo_box($prefix.'_metamap_content[]', $flag_options, FALSE, 0);
					echo '</td>';
					echo '<td class="sq-table-row">';
					text_box($prefix.'_metamap_metatag[]', '', 30);
					hidden_field($prefix.'_metamap_system[]', '0');
					echo '</td>';
					echo '<td class="sq-table-row">';
					check_box($prefix.'_metamap_delete['.($metadata_class_count+$i).']', '0');
					echo '</td>';
					echo '</tr>';
				}//end for
			} else {
				echo translate('Choose a collection to start editing');

			}//end if
		} else {
			echo translate('Choose a collection to start editing');

		}//end if

		return TRUE;

	}//end paintMetamap()


	/**
	* Processes the interface for editing metadata options
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processMetamap(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$collection_id = array_get_index($_POST, $prefix.'_metamap_collection_id', '');
			if (!empty($collection_id)) {
				$coll_conf_path = $asset->getCollectionDataPath($collection_id, 'conf');
				$metamap_config = $coll_conf_path.'/metamap.cfg';
				$data = Array();
				$alias = array_get_index($_POST, $prefix.'_metamap_alias', Array());
				$content = array_get_index($_POST, $prefix.'_metamap_content', Array());
				$metatags = array_get_index($_POST, $prefix.'_metamap_metatag', Array());
				$word_logic = array_get_index($_POST, $prefix.'_metamap_word_logic', Array());
				$system = array_get_index($_POST, $prefix.'_metamap_system', Array());
				$delete = array_get_index($_POST, $prefix.'_metamap_delete', Array());
				$metamap_word_logic = Array();

				foreach ($alias as $index => $mclass) {
					$metadata_class = $mclass;
					$metadata_index = array_get_index($content, $index, 0);
					$metadata_metag = array_get_index($metatags, $index, '');
					$metadata_system = array_get_index($system, $index, FALSE);
					$metadata_delete = array_get_index($delete, $index, '0');

					// Once the word logic for the particular metadata class is set,
					// use same value for other metadata in same class
					if (isset($metamap_word_logic[$mclass])) {
						$metadata_word_logic = $metamap_word_logic[$mclass];
					} else {
						 $metadata_word_logic = array_get_index($word_logic, $index, 2);
						 $metamap_word_logic[$mclass] = $metadata_word_logic;
					}

					if (!empty($metadata_metag) && !(!$metadata_system && $metadata_delete)) {
						$data[] = Array(
									'alias'			=> $metadata_class,
									'content'		=> $metadata_index,
									'metatag'		=> $metadata_metag,
									'word_logic'	=> $metadata_word_logic,
									'system'		=> $metadata_system,
								  );
					}//end if
				}//end foreach

				// Saving the attribute/file
				if (!empty($data)) {
					// Save
					$config = $asset->attr('metamap');
					$config[$collection_id] = $data;
					$asset->setAttrValue('metamap', $config);
					$status = $asset->saveAttributes();
					if ($status) {
						$status = $asset->writeConfiguration($metamap_config, $data, 'metadata', ',');
					}//end if
				}//end if
			}//end if
		}//end if

		return $status;

	}//end processMetamap()


// --- OTHER MISCELLANEOUS FUNCTION --- //


	/**
	* Paints the interface for the password field
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintPassword(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		require_once SQ_ATTRIBUTES_PATH.'/password/password.inc';
		$password_field = new Asset_Attribute_Password();
		$password_field->paint($prefix, !$wa);

		return TRUE;

	}//end paintPassword()


	/**
	* Processes the interface for the password field
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processPassword(&$asset, &$o, $prefix)
	{
		$status = FALSE;
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			require_once SQ_ATTRIBUTES_PATH.'/password/password.inc';
			$password_field = new Asset_Attribute_Password();
			$password_field->_edit_params['confirm_field'] = 0;
			$result = $password_field->process($prefix);

			if ($result !== FALSE && $password_field->processed == TRUE) {
				$asset->setAttrValue('password', $password_field->value);
				$status = $asset->saveAttributes();
			}//end if

			// Regardless of whether a password was entered or not, verify against the user
			// This only warns the user, it is up to them to change it!
			$user = $asset->attr('user');
			if (!empty($user)) {
				$user_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($user, '', TRUE);
				if (!is_null($user_asset)) {
					if (!$user_asset->comparePassword($asset->attr('password'))) {
						trigger_localised_error('FNB0014', translate('Invalid Password: Incorrect user or password, correct for proper use'), E_USER_WARNING)
						return FALSE;
					}//end if

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user_asset, TRUE);
				}//end if
			}//end if
		}//end if

		return $status;

	}//end processPassword()


// --- ASSET EXCLUSIONS SCREEN FUNCTIONS --- //


	/**
	* Paints the interface for choosing Asset IDs to exclude
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeAssetid(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$exclusions = array_get_index($asset->attr('global_exclude'), 'assetid', Array());
		if ($wa) {
			multiple_asset_finder($prefix.'_global_assetid_exclusions', $exclusions);
		} else {
			if (empty($exclusions)) {
				echo translate('No assets selected');

			} else {
				?><ul style="margin:1px 15px;"><?php
				foreach ($exclusions as $assetid) {
					echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				?></ul><?php
			}//end if
		}//end if

		return TRUE;

	}//end paintExcludeAssetid()


	/**
	* Processes the interface for processing Asset IDs to exclude
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processExcludeAssetid(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$exclusions = $asset->attr('global_exclude');
		$user_set = array_get_index($_POST, $prefix.'_global_assetid_exclusions', Array());

		if ($wa) {
			$global_assetid_exclude = Array();
			foreach ($user_set as $asset_info) {
				$assetid = array_get_index($asset_info, 'assetid', 0);
				if (empty($assetid)) continue;
				$global_assetid_exclude[] = $assetid;
			}//end foreach
			$exclusions['assetid'] = $global_assetid_exclude;
			$asset->setAttrValue('global_exclude', $exclusions);
		}//end if

		return TRUE;

	}//end processExcludeAssetid()


	/**
	* Paints the interface for choosing Asset Types to exclude
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeAssetType(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$exclusions = array_get_index($asset->attr('global_exclude'), 'type', Array());
		if ($wa) {
			asset_type_chooser($prefix.'_global_type_exclusions', TRUE, $exclusions);
		} else {
			// print the read-only version
			if (empty($exclusions)) {
				echo '<p class="sq-backend-warning">'.translate('No asset types selected').'</p>';

			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($exclusions, 'name');
				?><ul style="margin:1px 15px;"><?php
				foreach ($exclusions as $type) {
					echo '<li>';
					echo get_asset_type_icon($type);
					echo $type_names[$type];
					echo '</li>';
				}
				?></ul><?php
			}//end if
		}//end if

		return TRUE;

	}//end paintExcludeAssetType()


	/**
	* Processes the interface for processing Asset Types to exclude
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processExcludeAssetType(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$exclusions = $asset->attr('global_exclude');
		$user_set = array_get_index($_POST, $prefix.'_global_type_exclusions', Array());

		if ($wa) {
			$types = Array();
			foreach ($user_set as $type_code) {
				if (empty($type_code)) continue;
				$types[] = $type_code;
			}//end foreach
			$exclusions['type'] = $types;
			$asset->setAttrValue('global_exclude', $exclusions);
		}//end if

		return TRUE;

	}//end processExcludeAssetType()


	/**
	* Paints the interface for choosing Tree Node to exclude
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeTreeNode(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$exclusions = array_get_index($asset->attr('global_exclude'), 'tree', Array());
		if ($wa) {
			multiple_asset_finder($prefix.'_global_tree_exclusions', $exclusions);
		} else {
			if (empty($exclusions)) {
				echo translate('No root nodes selected');

			} else {
				?><ul style="margin:1px 15px;"><?php
				foreach ($exclusions as $assetid) {
					echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				?></ul><?php
			}//end if
		}//end if

		return TRUE;

	}//end paintExcludeTreeNode()


	/**
	* Processes the interface for processing Tree Node to exclude
	*
	* @param object	&$asset	the funnelback manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processExcludeTreeNode(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$exclusions = $asset->attr('global_exclude');
		$user_set = array_get_index($_POST, $prefix.'_global_tree_exclusions', Array());

		if ($wa) {
			$global_tree_exclude = Array();
			foreach ($user_set as $asset_info) {
				$assetid = array_get_index($asset_info, 'assetid', 0);
				if (empty($assetid)) continue;
				$global_tree_exclude[] = $assetid;
			}//end foreach
			$exclusions['tree'] = $global_tree_exclude;
			$asset->setAttrValue('global_exclude', $exclusions);
		}//end if

		return TRUE;

	}//end processExcludeTreeNode()


// --- PRIVATE FUNCTIONS --- //


	/**
	* Check if a collection is valid or not
	*
	* @param array	$collections	The collections to check
	* @param string	$collection_id	The collection id to check, if empty return all collections with valid set
	*
	* @return array|boolean
	* @access protected
	*/
	function _checkCollection($collections, $collection_id='')
	{
		foreach ($collections as $coll_id => $coll_info) {
			// If we are targeting a particular collection, skip over other collections
			if (!empty($collection_id) && $collection_id != $coll_id) {
				continue;
			}//end if

			// Go through the asset for this collection looking for a possible match
			$assetid = array_get_index($coll_info, 'asset', 0);
			$collection_url = array_get_index($coll_info, 'url', Array());
			if (!empty($assetid)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!is_null($asset)) {
					$poss_urls = $asset->getURLs();
					foreach ($poss_urls as $poss_url) {
						$http  = array_get_index($poss_url, 'http', FALSE);
						$https = array_get_index($poss_url, 'https', FALSE);
						$burl  = array_get_index($poss_url, 'url', '');
						if ($http && !empty($burl)) {
							$url_to_check = 'http://'.$burl;
							// If $url_to_check and url from the collection are the same, then this collection_id is valid
							if (in_array($url_to_check, $collection_url)) {
								$collections[$coll_id]['valid'] = TRUE;
							}//end if
						}//end if
						if ($https && !empty($burl)) {
							$url_to_check = 'https://'.$burl;
							// If $url_to_check and url from the collection are the same, then this collection_id is valid
							if (in_array($url_to_check, $collection_url)) {
								$collections[$coll_id]['valid'] = TRUE;
							}//end if
						}//end if
					}//end foreach
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($coll_asset, TRUE);
				}//end if
			}//end if
		}//end foreach

		if (!empty($collection_id)) {
			// Targetting a specific collection
			if (isset($collections[$collection_id]) && array_get_index($collections[$collection_id], 'valid', FALSE)) {
				return TRUE;
			} else {
				return FALSE;
			}//end if
		} else {
			// Not targetting a specific collection, return all collections
			return $collections;
		}//end if

	}//end _checkCollection()


	/**
	* Get collection defaults
	*
	* @param string	$type	The type collection we want the defaults for
	*
	* @return array
	* @access protected
	*/
	function _getCollectionDefault($type='web')
	{
		$default = Array(
					'name'		=> '',
					'settings'	=> Array(),
					'type'		=> $type,
				   );
		switch ($type) {
			case 'meta':
				$default['collections'] = Array();
			break;
			default:
		}//end switch

		return $default;

	}//end _getCollectionDefault()


	/**
	* Make changes to the feature page configuration.
	* This function is basically a wrapper function the funnelback_manager's writeConfiguration(),
	* just the main part of it is to filter the valid configuration lines.
	*
	* @param object	&$asset		The funnelback manager asset
	* @param string	$collection	The collection id
	* @param array	$data		The raw data to save
	*
	* @return void
	* @access protected
	*/
	function _writeFeaturePagesConfiguration(&$asset, $collection, $data)
	{
		$validData = Array();

		foreach ($data as $line) {
			$trigger = array_get_index($line, 'trigger', '');
			$url = array_get_index($line, 'url', '');
			$valid = array_get_index($line, 'valid', FALSE);
			if ($trigger != '' && $url != '' && $valid === TRUE) {
				// Found a valid line
				$validData[] = $line;
			}//end if
		}//end foreach

		$configFile = $asset->getCollectionDataPath($collection, 'conf').'/best_bets.cfg';
		$asset->writeConfiguration($configFile, $validData, 'feature', '==');

	}//end _writeFeaturePagesConfiguration()


}//end class
?>
