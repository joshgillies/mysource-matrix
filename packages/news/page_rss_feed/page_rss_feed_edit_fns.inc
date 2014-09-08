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
* $Id: page_rss_feed_edit_fns.inc,v 1.18 2012/08/30 00:59:01 ewang Exp $
*
*/
require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
/**
* Page_RSS_Feed_Edit_Fns
*
* Purpose
*
*
* @author  Pratibha Gannavarapu <pgannavarapu@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Page_RSS_Feed_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for choosing listing engine assets
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLisitngChooser(&$asset, &$o, $prefix)
	{
		$current_rss_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'listing_engine', FALSE, 'major', 'generate_rss');
		$rss_links = Array();
		foreach ($current_rss_links as $i => $link) {
			$rss_links[$link['minorid']] = $link['minor_type_code'];
		}
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			multiple_asset_finder($prefix.'_listing_engines', array_keys($rss_links), Array('listing_engine' => 'D'));
			return TRUE;
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($rss_links));
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('asset') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('type') ?></td>
				</tr>
			<?php
			foreach ($infos as $assetid => $info) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($assetid) ?></td>
				<td class="sq-backend-table-cell"><?php echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($info['type_code'], 'name') ?></td>
			</tr>
			<?php
			}
			?>
			</table>
			<?php
			return FALSE;
		}

	}//end paintLisitngChooser()


	/**
	* Process the interface for choosing the listing engine assets
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processLisitngChooser(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_listing_engines'])) {
			$listings = $_POST[$prefix.'_listing_engines'];
			$listing_ids = Array();
			foreach ($listings as $listing) {
				if (!empty($listing['assetid'])) {
					$listing_ids[] = $listing['assetid'];
				}
			}
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'listing_engine', FALSE, 'major', 'generate_rss');
			$existing_list_ids = Array();
			foreach ($links as $link) {
				if (!in_array($link['minorid'], $listing_ids)) {
					$asset->deleteLink($link['linkid']);
				} else {
					$existing_list_ids[] = $link['minorid'];
				}
			}
			foreach ($listing_ids as $listing_id) {
				if (!in_array($listing_id, $existing_list_ids)) {
					$listing_engine = $GLOBALS['SQ_SYSTEM']->am->getAsset($listing_id);
					$id = $asset->createLink($listing_engine, SQ_LINK_NOTICE, 'generate_rss');
				}
			}
		}

	}//end processLisitngChooser()


	/**
	* Paints the interface for the use to select keywords
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintKeywords(&$asset, &$o, $prefix)
	{
		$types = Array();
		$rss_source_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'listing_engine', FALSE, 'major', 'generate_rss');
		if (!empty($rss_source_links)) {
			$keywords = Array();
			foreach ($rss_source_links as $links) {
				$listing_engine = $GLOBALS['SQ_SYSTEM']->am->getAsset($links['minorid']);
				$attribute_types =  array_keys($listing_engine->attr('types'));
				if (!empty($attribute_types)) {
					foreach ($attribute_types as $type) {
						if (!isset($keywords[$type])) {
							$keywords[$type] = $asset->_getAvailableKeywordsOfType($type);
						}
						$types[$type] = $type;
					}
				}
			}
			$types['select'] = '-- '.translate('select_asset_type').' --';
			$wa = $asset->writeAccess('attributes');
			if ($wa) {
				$type = null;
				$tag = null;
				if (isset($_POST['asset_type']) && ($_POST['asset_type'] != 'select')) {
					$type = $_POST['asset_type'];
				}
				if (isset($_POST[$prefix.'_tag']) && !empty($_POST[$prefix.'_tag'])) {
					$tag = $_POST[$prefix.'_tag'];
					$type = null;
				}

				?>
				<table class="sq-backend-table">
					<tr>
						<td> <?php echo translate('choose_asset_type');?></td>
						<td>
							<?php
							combo_box('asset_type', $types, FALSE, !empty($type)? $type: 'select');
							?>
						</td>
						<td>&nbsp;</td>
					</tr>
					<?php
					if (!empty($type)) {
						$keywords['select'] = '-- '.translate('select_keyword').' --';
						$keyword = null;
						$tag = null;
						if (isset($_POST[$prefix.'_keyword']) && ($_POST[$prefix.'_keyword'] != 'select')) {
							$keyword = $_POST[$prefix.'_keyword'];
						}
						if (empty($tag)) {
							?>
							<tr>
								<td><?php echo translate('choose_or_enter_keyword');?></td>
								<td>
									<?php
									combo_box($prefix.'_keyword', $keywords[$type], FALSE, !empty($keyword)? $keyword: 'select');
									?>
								</td>
								<td>
									<?php
									text_box($prefix.'_keyword_text', '');
									?>
								</td>
							</tr>
							<?php
							$type = $asset->attr('rss_types');
							$tags = Array();
							if ($type == 'rss_1.0') {
								$tags = Array('title', 'description', 'link', 'rdf:about');
							} else if ($type == 'rss_2.0') {
								$tags = Array('title', 'description', 'link', 'pubDate', 'category', 'author', 'comments', 'guid', 'enclosure.url', 'enclosure.length', 'enclosure.type');
							} else {
								// it is of type ATOM 1.0
								// Refer http://www.atomenabled.org/developers/syndication/#requiredEntryElements
								$tags = Array('id', 'title', 'updated', 'author_name', 'author_email', 'author_uri', 'content', 'link.rel', 'link.href', 'summary', 'category.term', 'contributor_name', 'contributor_email', 'contributor_uri', 'source_id', 'source_title', 'source_updated', 'source_rights', 'rights');
							}
							foreach ($tags as $key => $val) {
								$tags[$val] = strpos($val, '_') !== FALSE? str_replace('_', ' ', $val): (strpos($val, '.') !== FALSE? str_replace('.', ' ', $val).' = ':$val);
								unset($tags[$key]);
							}
							?>
							<tr>
								<td><?php echo translate('choose_tag');?></td>
								<td>
									<?php
									combo_box($prefix.'_tag', $tags, FALSE, !empty($tag)? $tag: '');
									?>
								</td>
								<td>&nbsp;</td>
							</tr>
						<?php
						}//end if tag is not empty
					}//end if type is not empty
				?>
				</table>
				<?php
				return TRUE;
			}//end if write access
		} else {
			echo 'No Listing Engine Assets Have Been Selected To Generate The RSS';
		}
		return FALSE;

	}//end paintKeywords()


	/**
	* Process the interface for the user to select keywords
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processKeywords(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_tag']) && !empty($_POST[$prefix.'_tag'])) {
			$type = $_POST['asset_type'];
			if ($type != 'select') {
				$tag = $_POST[$prefix.'_tag'];
				$keyword = null;
				if (empty($_POST[$prefix.'_keyword_text'])) {
					$keyword = $_POST[$prefix.'_keyword'];
				} else {
					$keyword = $_POST[$prefix.'_keyword_text'];
				}
				$items = $asset->attr('items');
				$items[$type][$tag] = $keyword;
				$asset->setAttrValue('items', $items);
				return TRUE;
			}
		}
		return FALSE;

	}//end processKeywords()


	/**
	* Paints the interface for the selected keywords
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSelectedKeywords(&$asset, &$o, $prefix)
	{
		$items = $asset->attr('items');
		$wa = $asset->writeAccess('attributes');
		if (!empty($items)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('asset_type');?></th>
					<th><?php echo translate('keyword');?></th>
					<th><?php echo translate('tag_represented');?></th>
					<?php
					if ($wa) {
						?>
						<th><?php echo translate('delete_question');?></th>
					<?php
					}
					?>
				</tr>
				<?php
				foreach ($items as $type => $info) {
					foreach ($info as $tag => $keyword) {
						?>
						<tr>
							<td><?php echo $type;?></td>
							<td><?php echo $keyword; ?></td>
							<td><?php echo $tag; ?></td>
							<?php
							if ($wa) {
								?>
								<td><?php check_box('delete_tag['.$type.']['.$tag.']', $keyword);?></td>
							<?php
							}
						?>
						</tr>
					<?php
					}
				}
			?>
			</table>
		<?php
		}//end if where items is not empty
		if ($wa) return TRUE;
		return FALSE;

	}//end paintSelectedKeywords()


	/**
	* Process the interface for the user to select keywords
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSelectedKeywords(&$asset, &$o, $prefix)
	{
		if (isset($_POST['delete_tag'])) {
			$selected_items = $asset->attr('items');
			$deleted = $_POST['delete_tag'];
			foreach ($deleted as $type => $info) {
				foreach ($info as $tag => $keyword) {
					unset($selected_items[$type][$tag]);
				}
			}
			// if there are no keywords and tags for a particular asset type then unset the asset type from the items array
			// you dont want to have a key in an array with an empty value.. no !! never.. unless i want a messy interface
			foreach ($selected_items as $key => $value) {
				if (empty($value)) unset($selected_items[$key]);
			}
			$asset->setAttrValue('items', $selected_items);
			return TRUE;
		}
		return FALSE;

	}//end processSelectedKeywords()


	/**
	* Paints the interface for entering the channel info or image info
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintChannelImage(&$asset, &$o, $prefix)
	{
		$modes = Array('add_channel', 'add_image');
		foreach ($modes as $mode) {
			$tags = Array();
			$required = Array();
			$type = $asset->attr('rss_types');
			switch ($mode) {
				case 'add_channel':
					if ($type == 'rss_1.0') {
						$tags = Array('title', 'link', 'description', 'rdf:about');
						$required = $tags;
					} else if ($type == 'rss_2.0') {
						$required = Array('title', 'link', 'description');
						$tags = Array('title', 'link', 'description', 'pubDate', 'lastBuildDate', 'language', 'copyright', 'webMaster', 'managingEditor', 'category', 'generator', 'ttl');
					} else {
						// type is ATOM 1.0
						// Refer http://www.atomenabled.org/developers/syndication/#requiredFeedElements
						$required = Array('id', 'title', 'updated');
						$recommended = Array('author_name', 'author_email', 'author_uri', 'link.rel', 'link.href');
						$optional = Array('category.term', 'contributor_name', 'icon', 'logo', 'rights', 'subtitle');
						$tags = array_merge($required, $recommended, $optional);
					}
				break;
				case 'add_image':
					if ($type == 'rss_1.0') {
						$tags = Array('title', 'url', 'link');
						$required = Array('title', 'url', 'link');
					} else if ($type == 'rss_2.0') {
						$tags = Array('title', 'description', 'url', 'width', 'height', 'link');
						$required = Array('title', 'url', 'link');
					} else {
						// ATOM 1.0
						$tags = Array('icon');
						$required = Array('icon');
					}
				break;
			}
			$attr = substr($mode, strlen('add_'));
			$attr_val = $asset->attr($attr);
			$wa = $asset->writeAccess('attributes');
			$i = 0;
			if (!empty($attr_val) || ($wa)) {
				echo '<br /><b>'.translate($attr.'_info').'</b>';
				?>
				<table class="sq-backend-table" width="100%">
					<tr>
						<th><?php echo translate('tag');?></th>
						<th><?php echo translate('value');?></th>
						<?php
						if (($wa) && (in_array('link', $tags) || in_array('link.href', $tags))) {
							?>
							<th><?php echo translate('link_text_value');?></th>
						<?php
						}
						?>
						<th><?php echo translate('required');?></th>
					</tr>
					<?php
					foreach ($tags as $tag) {
						?>
						<tr>
							<td><?php echo $tag; ?></td>
							<td>
								<?php
								if ($wa) {
									// if the tag is not a link then just use the value normally
									// else check if the value is a valid asset id and if yes then print the asset finder with the assetid
									// else just print the value normally in a text box
									if (($tag != 'link') && ($tag !== 'link.href')) {
										text_box($attr.'['.$tag.']', isset($attr_val[$tag])? $attr_val[$tag]: '', 30);
										if (($wa) && (in_array('link', $tags) || in_array('link.href', $tags))) {
											?>
											</td>
											<td>&nbsp;
											<?php
										}
									} else {
										$id = 0;
										$url = null;
										if (isset($attr_val[$tag])) {
											if (assert_valid_assetid($attr_val[$tag], '', TRUE, FALSE)) {
												$id = $attr_val[$tag];
											} else {
												$url = $attr_val[$tag];
											}
										}
										asset_finder($attr.'['.$tag.']', $id);
										?>
										</td>
										<td><?php text_box($attr.'['.$tag.'_text]', $url);
									}
								} else {
									// if the tag is not a link then just use the value normally
									// else check if the value is a valid asset id and if yes then print the asset's url
									// else just print the value normally
									if (($tag != 'link') && ($tag !== 'link.href')) {
										echo isset($attr_val[$tag])? $attr_val[$tag]: '&nbsp;';
									} else if (isset($attr_val[$tag])) {
										if (assert_valid_assetid($attr_val[$tag], '', TRUE, FALSE)) {
											$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($attr_val[$tag]);
											echo $value;
										} else {
											echo $attr_val[$tag];
										}
									} else {
										echo '&nbsp;';
									}
								}
							?>
							</td>
							<td>
								<?php
								// if the current tag is a required tag or not
								if (!empty($required)) {
									if (in_array($tag, $required)) {
										echo 'Yes';
									} else {
										echo 'No';
									}
								}
								?></td>
						</tr>
						<?php
					}//end foreach of tags
					?>
				</table>
			<?php
			}//end if write access or attribute value is not empty
		}//end foreach of modes

	}//end paintChannelImage()


	/**
	* Process the interface for entering the channel info or image info
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processChannelImage(&$asset, &$o, $prefix)
	{
		$modes = Array('add_channel', 'add_image');
		foreach ($modes as $mode) {
			$attr = substr($mode, strlen('add_'));
			if (isset($_POST[$attr])) {
				$attr_val = $_POST[$attr];
				$new_val = Array();
				foreach ($attr_val as $tag => $value) {
					if ($tag == 'link_text') continue;
					if (!empty($value)) {
						if (in_array($tag, Array('link', 'link.href'))) {
							// if there is a text value for the link then it is taken
							// else the asset id is taken and its url is stored
							if (!empty($attr_val['link_text'])) {
								$value = $attr_val['link_text'];
							} else {
								if (empty($value['assetid'])) continue;
								$value = $value['assetid'];
							}
						}
						$new_val[$tag] = $value;
					}
				}
				$asset->setAttrValue($attr, $new_val);
			}
		}
		return TRUE;

	}//end processChannelImage()


	/**
	* Find out if we have write access to asset attributes
	*
	* @param object	&$asset	The page_rss_feed asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function ifWriteAccess(&$asset, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) return TRUE;
		return FALSE;

	}//end ifWriteAccess()


	/**
	* Find out if we there is any items
	*
	* @param object	&$asset	The page_rss_feed asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function ItemsNotEmpty(&$asset, $prefix)
	{
		$items = $asset->attr('items');
		if (!empty($items)) return TRUE;
		return FALSE;

	}//end ItemsNotEmpty()


}//end class
?>
