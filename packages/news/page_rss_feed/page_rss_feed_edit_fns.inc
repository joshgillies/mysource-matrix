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
* $Id: page_rss_feed_edit_fns.inc,v 1.1 2005/11/01 22:24:01 pgannavarapu Exp $
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
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Page_RSS_Feed_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_RSS_Feed_Edit_Fns()
	{
		$this->Page_Edit_Fns();

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
		$current_rss_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'listing_engine', false, 'major', 'generate_rss');
		$wa = $asset->writeAccess('attributes');
		?>
		<table>
			<?php
			$i = 0;
			if (!empty($current_rss_links)) {
				foreach ($current_rss_links as $i => $links) {
					?>
					<tr><td>
						<?php
						if ($wa) {
							asset_finder($prefix.'_listing_engines['.$i.']', $links['minorid'], Array('listing_engine' => 'D'));
						} else {
							echo $links['minorid'];
						}
					?>
					</td></tr>
				<?php
				}
			} else {
				?>
				<tr><td><?php echo 'No Listing Engine Assets Selected';?></td></tr>
				<?php
			}
			if ($wa) {
				?>
				<tr><td>
					<?php
					$i++;
					asset_finder($prefix.'_listing_engines['.$i.']', 0, Array('listing_engine' => 'D'));
					?>
				</td></tr>
			<?php
			}
		?>
		</table>
		<?php
		if ($wa) return true;
		return false;

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
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'listing_engine', false, 'major', 'generate_rss');
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
					$listing_engine =& $GLOBALS['SQ_SYSTEM']->am->getAsset($listing_id);
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
		$rss_source_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'listing_engine', false, 'major', 'generate_rss');
		if (!empty($rss_source_links)) {
			$keywords = Array();
			foreach ($rss_source_links as $links) {
				$listing_engine =& $GLOBALS['SQ_SYSTEM']->am->getAsset($links['minorid']);
				$types = array_keys($listing_engine->attr('types'));
				if (!empty($types)) {
					foreach ($types as $type) {
						if (!isset($keywords[$type])) {
							$keywords[$type] = $asset->_getAvailableKeywordsOfType($type);
						}
					}
				}
			}
			foreach ($types as $key => $type) {
				$types[$type] = $type;
				unset($types[$key]);
			}
			$types['select'] = translate('select_asset_type');
			$wa = $asset->writeAccess('attributes');
			if ($wa) {
				$type = null;
				if (isset($_POST['asset_type']) && ($_POST['asset_type'] != 'select')) {
					$type = $_POST['asset_type'];
				}
				?>
				<table class="sq-backend-table">
					<tr>
						<td> <?php echo translate('choose_asset_type');?></td>
						<td>
							<?php
							combo_box('asset_type', $types, false, !empty($type)? $type: 'select');
							?>
						</td>
						<td>&nbsp;</td>
					</tr>
					<?php
					if (!empty($type)) {
						$keywords['select'] = translate('select_keyword');
						$keyword = null;
						$tag = null;
						if (isset($_POST[$prefix.'_keyword']) && ($_POST[$prefix.'_keyword'] != 'select')) {
							$keyword = $_POST[$prefix.'_keyword'];
						}
						if (isset($_POST[$prefix.'_tag']) && !empty($_POST[$prefix.'_tag'])) {
							$tag = $_POST[$prefix.'_tag'];
						}
						if (empty($tag)) {
							?>
							<tr>
								<td><?php echo translate('choose_or_enter_keyword');?></td>
								<td>
									<?php
									combo_box($prefix.'_keyword', $keywords[$type], false, !empty($keyword)? $keyword: 'select');
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
								$tags = Array('title', 'description', 'link');
							} else if ($type == 'rss_2.0') {
								$tags = Array('title', 'description', 'link', 'pubDate', 'category', 'author', 'comments', 'guid', 'enclosure');
							} else {
								//it is of type ATOM 1.0
								$tags = Array('title', 'content', 'link.rel', 'link.type', 'link.href', 'category.term', 'category.label',  'id', 'contributor_name', 'author_name', 'author_email', 'author_url', 'summary', 'published', 'updated', 'source_id', 'source_title', 'source_rights');
							}
							foreach ($tags as $key => $val) {
								$tags[$val] = strpos($val, '_') !== false? str_replace('_', ' ', $val): (strpos($val, '.') !== false? str_replace('.', ' ', $val).' = ':$val);
								unset($tags[$key]);
							}
							?>
							<tr>
								<td><?php echo translate('choose_tag');?></td>
								<td>
									<?php
									combo_box($prefix.'_tag', $tags, false, !empty($tag)? $tag: '');
									?>
								</td>
								<td>&nbsp;</td>
							</tr>
						<?php
						}
					}
				?>
				</table>
				<?php
				return true;
			}
		} else {
			echo 'No Listing Engine Assets Have Been Selected To Generate The RSS';
		}
		return false;

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
				return true;
			}
		}
		return false;

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
		}
		if ($wa) return true;
		return false;

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
			//$selected_items = Array();
			$asset->setAttrValue('items', $selected_items);
			return true;
		}
		return false;

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
			$type = $asset->attr('rss_types');
			switch ($mode) {
				case 'add_channel':
					if ($type == 'rss_1.0') {
						$tags = Array('title', 'link', 'description');
					} else if ($type == 'rss_2.0') {
						$tags = Array('title', 'link', 'description', 'pubDate', 'lastBuildDate', 'language', 'copyright', 'webmaster', 'managingEditor', 'category', 'generator', 'ttl', 'docs');
					} else {
						//type is ATOM 1.0
						$tags = Array('title', 'id', 'link.rel', 'link.type', 'link.href', 'subtitle', 'updated', 'rights', 'author_name', 'author_email', 'author_url', 'category.term', 'category.label', 'category.scheme', 'rights');
					}
				break;
				case 'add_image':
					if ($type == 'rss_1.0') {
						$tags = Array('title', 'url', 'link');
					} else if ($type == 'rss_2.0') {
						$tags = Array('title', 'description', 'url', 'weight', 'height', 'link');
					} else {
						//ATOM 1.0
						$tags = Array('icon');
					}
				break;
			}
			foreach ($tags as $key => $val) {
				$tags[$val] = strpos($val, '_') !== false? str_replace('_', ' ', $val): (strpos($val, '.') !== false? str_replace('.', ' ', $val).' = ':$val);
				unset($tags[$key]);
			}
			$attr = substr($mode, strlen('add_'));
			$attr_val = $asset->attr($attr);
			$wa = $asset->writeAccess('attributes');
			$i = 0;
			if (!empty($attr_val)) {
				echo '<br /><b>'.translate('current_'.$attr.'_info').'</b>';
				?>
				<table class="sq-backend-table">
					<tr>
						<th><?php echo translate('tag');?></th>
						<th><?php echo translate('value');?></th>
						<?php
						if ($wa) {
							?>
							<th><?php echo translate('delete_question');?></th>
						<?php
						}
					?>
					</tr>
					<?php
					foreach ($attr_val as $channel) {
						?>
						<tr>
							<?php
							if ($wa) {
								?>
								<td><?php text_box($attr.'['.$i.'][tag]', $channel['tag']);?></td>
								<td><?php text_box($attr.'['.$i.'][value]', $channel['value']);?></td>
								<td><?php check_box($attr.'['.$i.'][delete]', $channel['tag']);?></td>
							<?php
							} else {
								?>
								<td><?php echo $channel['tag'];?></td>
								<td><?php echo $channel['value'];?></td>
							<?php
							}
						?>
						</tr>
						<?php
						$i++;
					}
				?>
				</table>
			<?php
			}
			if ($wa) {
				echo '<br /><b>'.translate('add_new_'.$attr.'_info').'</b>';
				?>
				<table class="sq-backend-table">
					<tr>
						<th><?php echo translate('tag');?></th>
						<th><?php echo translate('value');?></th>
					</tr>
					<tr>
						<td><?php combo_box($attr.'['.$i.'][tag]', $tags, false, '');?></td>
						<td><?php text_box($attr.'['.$i.'][value]', '');?></td>
					</tr>
				</table>
				<?php
			}
		}

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
				foreach ($attr_val as $channel) {
					if (!isset($channel['delete']) && !empty($channel['tag']) && !empty($channel['value'])) {
						$new_val[] = $channel;
					}
				}
				$asset->setAttrValue($attr, $new_val);
			}
		}
		return true;

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
		if ($wa) return true;
		return false;

	}//end ifWriteAccess()


}//end class
?>
