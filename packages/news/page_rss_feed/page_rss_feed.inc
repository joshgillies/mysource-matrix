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
* $Id: page_rss_feed.inc,v 1.5 2006/01/04 22:06:51 rong Exp $
*
*/



/**
* Page_RSS_Feed
*
* Purpose
*
*
* @author  Pratibha Gannavarapu <pgannavarapu@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
	require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
	require_once SQ_LIB_PATH.'/html_form/html_form.inc';


class Page_RSS_Feed extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function Page_RSS_Feed($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('name'));

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	* @see Asset::_getAllowLinks()
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$links = Array();
		$links[SQ_LINK_NOTICE]['listing_engine']    = Array('card' => 'M', 'exclusive' => false);
		return $links;

	}//end _getAllowedLinks()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset listings create bopycopy and folder asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		return $this->saveWebPaths($good_paths);

	}//end _createAdditional()


	/**
	* Function that takes asset type as the argument and returns an array of the avaialble keywords for that asset type
	*
	* @param string	$type	the asset type
	*
	* @return array
	* @access private
	*/
	function _getAvailableKeywordsOfType($type='')
	{
		$keywords = Array();
		if (empty($type)) return $keywords;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
		$dummy_asset =& new $type();
		$keywords += $dummy_asset->getAvailableKeywords();
		unset($dummy_asset);
		return $keywords;

	}//end _getAvailableKeywordsOfType()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'generate_rss' :
				return translate('listing_engine_asset_link_description');
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}
		$rss = '';
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$rss = $cm->loadFromCache($this->id, $this->type(), '', false);
		$rss = unserialize($rss);
		if (empty($rss)) {
			$assets_to_list = Array();
			$rss_source_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'listing_engine', false, 'major', 'generate_rss');
			if (empty($rss_source_links)) {
				trigger_error(translate('no_listing_engine_assets_linked'), E_USER_WARNING);
				return;
			}
			foreach ($rss_source_links as $links) {
				$listing_engine =& $GLOBALS['SQ_SYSTEM']->am->getAsset($links['minorid']);
				$assets_to_list += $listing_engine->getAssetList();
			}

			$channel = $this->attr('channel');
			$image = $this->attr('image');
			$items = $this->attr('items');
			if (empty($channel) || empty($items)) {
				trigger_error(translate('no_channel_or_items_found'), E_USER_WARNING);
				return;
			} else if (empty($assets_to_list)) {
				trigger_error(translate('no_assets_found_for_items'), E_USER_WARNING);
				return;
			} else {
				require_once SQ_FUDGE_PATH.'/rss_feeds/rss_feed_writer.inc';
				$writer =& new RSS_Feed_Writer();
				$type = $this->attr('rss_types');
				$type = explode('_', $type);
				$writer->setTypeVersion($type[0], $type[1]);
				$new_channel = Array();
				$rss_type = implode('_', $type);
				foreach ($channel as $tag => $value) {
					if (strpos($tag, '_') !== false) {
						$sub_tag = explode('_', $tag);
						$new_channel[$sub_tag[0]]['SUBTAGS'][$sub_tag[1]] = $value;
					} else if (strpos($tag, '.') !== false) {
						if ($tag == 'link.href') {
							if (assert_valid_assetid($value, '', true, false)) {
								$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
							}
						}
						$sub_tag = explode('.', $tag);
						$new_channel[$sub_tag[0]]['ATTRIBUTES'][$sub_tag[1]] = $value;
					} else {
						//converting date values into the respective formats
						if (in_array($tag, Array('updated', 'pubDate', 'lastBuildDate', 'published'))) {
							$timestamp = strtotime($value);
							if (($timestamp == -1) ||($timestamp === false)) {
								$timestamp = mktime($value);
							}
							if ($rss_type == 'rss_2.0') {
								$value = date('D, j M Y H:i:s T', $timestamp);
							} else if ($rss_type == 'atom_1.0') {
								$value = date('Y-m-j', $timestamp).'T'.date('H:i:s', $timestamp).'Z';
							}
						} else {
							if ($tag == 'link') {
								if (assert_valid_assetid($value, '', true, false)) {
									$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
								}
							}
						}
						$new_channel[$tag] = $value;
					}
				}
				$no_errors = $this->validateChannel($new_channel);
				if (!$no_errors) return;
				//adding channel info to the writer
				$writer->addChannel($new_channel);
				$new_image = Array();
				foreach ($image as $tag => $value) {
					if (strpos($tag, '_') !== false) {
						$sub_tag = explode('_', $tag);
						$new_image[$sub_tag[0]][$sub_tag[1]] = $value;
					} else {
						if ($tag == 'link') {
							if (assert_valid_assetid($value, '', true, false)) {
								$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
							}
						}
						$new_image[$tag] = $value;
					}
				}
				if (!empty($new_image)) {
					$no_errors = $this->validateImage($new_image);
					if (!$no_errors) return;
					//adding image details to the writer
					$writer->addImage($new_image);
				}
				$new_items = Array();
				$index = 0;
				$mm = null;
				//now we have to add items
				foreach ($items as $type => $keywords) {
					foreach ($assets_to_list as $assetid => $asset_type) {
						$asset = null;
						if (strcasecmp($type, $asset_type) == 0) {
							$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);
							foreach ($keywords as $tag => $keyword) {
								if ($asset) {
									$replacement = $asset->getKeywordReplacement($keyword);
									if (substr($keyword, 0, 15) == 'asset_metadata_') {
										$metadata_keyword = substr($keyword, 15);
										if (is_null($mm)) {
											$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
										}
										// get all the metadata keywords for this asset
										$metadata_values = $mm->getMetadataFieldValues($assetid, Array($metadata_keyword));

										foreach ($metadata_values as $field => $value) {
											$replacement = $value;
										}
									}
									if (strpos($tag, '_') !== false) {
										$sub_tag = explode('_', $tag);
										$new_items[$index][$sub_tag[0]]['SUBTAGS'][$sub_tag[1]] = $replacement;
									} else if (strpos($tag, '.') !== false) {
										$sub_tag = explode('.', $tag);
										$new_items[$index][$sub_tag[0]]['ATTRIBUTES'][$sub_tag[1]] = $replacement;
									} else {
										//converting date values into the respective formats
										if (in_array($tag, Array('updated', 'pubDate', 'lastBuildDate', 'published'))) {
											$timestamp = strtotime($replacement);
											if ($rss_type == 'rss_2.0') {
												$replacement = date('D, j M Y G:i:s T', $timestamp);
											} else if ($rss_type == 'atom_1.0') {
												$replacement = date('Y-m-d', $timestamp).'T'.date('G:i:s', $timestamp).'Z';
											}
										}
										$new_items[$index][$tag] = $replacement;
									}
								}
							}
							$index++;
						}
					}
				}
				$no_errors = $this->validateItems($new_items);
				if (!$no_errors) return;
				$writer->addItems($new_items);
				$rss = $writer->getRSSString();
				$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
				$r = $cm->saveToCache($this->id, $this->type(), '', serialize($rss), false);
			}
		} else {
			//we need to send headers as we got the feed from cache
			header('Content-type: text/xml');
		}
		echo $rss;

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$rss = '';
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$rss = $cm->loadFromCache($this->id, $this->type(), '', false);
		$rss = unserialize($rss);
		if (empty($rss)) {
			$assets_to_list = Array();
			$rss_source_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'listing_engine', false, 'major', 'generate_rss');
			if (empty($rss_source_links)) {
				trigger_error(translate('no_listing_engine_assets_linked'), E_USER_WARNING);
				return;
			}
			foreach ($rss_source_links as $links) {
				$listing_engine =& $GLOBALS['SQ_SYSTEM']->am->getAsset($links['minorid']);
				$assets_to_list += $listing_engine->getAssetList();
			}

			$channel = $this->attr('channel');
			$image = $this->attr('image');
			$items = $this->attr('items');
			if (empty($channel) || empty($items)) {
				trigger_error(translate('no_channel_or_items_found'), E_USER_WARNING);
				return;
			} else if (empty($assets_to_list)) {
				trigger_error(translate('no_assets_found_for_items'), E_USER_WARNING);
				return;
			} else {
				require_once SQ_FUDGE_PATH.'/rss_feeds/rss_feed_writer.inc';
				$writer =& new RSS_Feed_Writer();
				$type = $this->attr('rss_types');
				$type = explode('_', $type);
				$writer->setTypeVersion($type[0], $type[1]);
				$new_channel = Array();
				$rss_type = implode('_', $type);
				foreach ($channel as $tag => $value) {
					if (strpos($tag, '_') !== false) {
						$sub_tag = explode('_', $tag);
						$new_channel[$sub_tag[0]]['SUBTAGS'][$sub_tag[1]] = $value;
					} else if (strpos($tag, '.') !== false) {
						if ($tag == 'link.href') {
							if (assert_valid_assetid($value, '', true, false)) {
								$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
							}
						}
						$sub_tag = explode('.', $tag);
						$new_channel[$sub_tag[0]]['ATTRIBUTES'][$sub_tag[1]] = $value;
					} else {
						//converting date values into the respective formats
						if (in_array($tag, Array('updated', 'pubDate', 'lastBuildDate', 'published'))) {
							$timestamp = strtotime($value);
							if (($timestamp == -1) ||($timestamp === false)) {
								$timestamp = mktime($value);
							}
							if ($rss_type == 'rss_2.0') {
								$value = date('D, j M Y H:i:s T', $timestamp);
							} else if ($rss_type == 'atom_1.0') {
								$value = date('Y-m-j', $timestamp).'T'.date('H:i:s', $timestamp).'Z';
							}
						} else {
							if ($tag == 'link') {
								if (assert_valid_assetid($value, '', true, false)) {
									$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
								}
							}
						}
						$new_channel[$tag] = $value;
					}
				}
				$no_errors = $this->validateChannel($new_channel);
				if (!$no_errors) return;
				//adding channel info to the writer
				$writer->addChannel($new_channel);
				$new_image = Array();
				foreach ($image as $tag => $value) {
					if (strpos($tag, '_') !== false) {
						$sub_tag = explode('_', $tag);
						$new_image[$sub_tag[0]][$sub_tag[1]] = $value;
					} else {
						if ($tag == 'link') {
							if (assert_valid_assetid($value, '', true, false)) {
								$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
							}
						}
						$new_image[$tag] = $value;
					}
				}
				if (!empty($new_image)) {
					$no_errors = $this->validateImage($new_image);
					if (!$no_errors) return;
					//adding image details to the writer
					$writer->addImage($new_image);
				}
				$new_items = Array();
				$index = 0;
				$mm = null;
				//now we have to add items
				foreach ($items as $type => $keywords) {
					foreach ($assets_to_list as $assetid => $asset_type) {
						$asset = null;
						if (strcasecmp($type, $asset_type) == 0) {
							$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);
							foreach ($keywords as $tag => $keyword) {
								if ($asset) {
									$replacement = $asset->getKeywordReplacement($keyword);
									if (substr($keyword, 0, 15) == 'asset_metadata_') {
										$metadata_keyword = substr($keyword, 15);
										if (is_null($mm)) {
											$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
										}
										// get all the metadata keywords for this asset
										$metadata_values = $mm->getMetadataFieldValues($assetid, Array($metadata_keyword));

										foreach ($metadata_values as $field => $value) {
											$replacement = $value;
										}
									}
									if (strpos($tag, '_') !== false) {
										$sub_tag = explode('_', $tag);
										$new_items[$index][$sub_tag[0]]['SUBTAGS'][$sub_tag[1]] = $replacement;
									} else if (strpos($tag, '.') !== false) {
										$sub_tag = explode('.', $tag);
										$new_items[$index][$sub_tag[0]]['ATTRIBUTES'][$sub_tag[1]] = $replacement;
									} else {
										//converting date values into the respective formats
										if (in_array($tag, Array('updated', 'pubDate', 'lastBuildDate', 'published'))) {
											$timestamp = strtotime($replacement);
											if ($rss_type == 'rss_2.0') {
												$replacement = date('D, j M Y G:i:s T', $timestamp);
											} else if ($rss_type == 'atom_1.0') {
												$replacement = date('Y-m-d', $timestamp).'T'.date('G:i:s', $timestamp).'Z';
											}
										}
										$new_items[$index][$tag] = $replacement;
									}
								}
							}
							$index++;
						}
					}
				}
				$no_errors = $this->validateItems($new_items);
				if (!$no_errors) return;
				$writer->addItems($new_items);
				$rss = $writer->getRSSString();
				$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
				$r = $cm->saveToCache($this->id, $this->type(), '', serialize($rss), false);
			}
		} else {
			//we need to send headers as we got the feed from cache
			header('Content-type: text/xml');
		}
		echo $rss;

	}//end printBody()


	/**
	* Checks if the passed items have all of the required tags or not and returns true if yes
	*
	* @param array	$items	itemso that the user has created for this feed
	*
	* @return boolean
	* @access public
	*/
	function validateItems($items=Array())
	{
		if (empty($items)) {
			trigger_error('Missing Items', E_USER_WARNING);
			return false;
		} else {
			$required = Array();
			$rss_type = $this->attr('rss_types');
			switch ($rss_type) {
				case 'atom_1.0':
					$required = Array('id', 'updated', 'title');
				break;
				case 'rss_1.0':
					$required = Array('link', 'title');
				break;
				case 'rss_2.0':
				break;
			}
			foreach ($items as $item) {
				$keys = array_keys($item);
				if (in_array('category', $keys)) {
					$temp = $items['category']['ATTRIBUTUES'];
					if (!in_array('term', array_keys($temp))) {
						trigger_error(translate('required_attr_term_unassigned'), E_USER_WARNING);
						return false;
					}
				}
				if ($rss_type == 'rss_2.0') {
					if (!in_array('title', $keys)) {
						if (!in_array('description', $keys)) {
							trigger_error(translate('required_tag_title_or_description_missing'), E_USER_WARNING);
							return false;
						}
					}
				}
				foreach ($required as $req) {
					if (!in_array($req, $keys)) {
						trigger_error('Required Tag '.$req.' Is Not Assigned In Items', E_USER_WARNING);
						return false;
					}
				}
			}
			return true;
		}

	}//end validateItems()


	/**
	* Checks if the passed channel information has all of the required tags or not and returns true if yes
	*
	* @param array	$channel	channel info that the user has created for this feed
	*
	* @return boolean
	* @access public
	*/
	function validateChannel($channel=Array())
	{
		if (empty($channel)) {
			trigger_error('Missing Channel Information', E_USER_WARNING);
			return false;
		}
		$required = Array();
		$rss_type = $this->attr('rss_types');
		if ($rss_type != 'atom_1.0') {
			$required = Array('title', 'link', 'description');
		} else {
			$required = Array('title', 'id', 'updated');
		}
		foreach ($required as $tag) {
			if (!in_array($tag, array_keys($channel))) {
				trigger_error(translate('required_attribute').' "'.$tag.'" '.translate('missing_for_channel'), E_USER_WARNING);
				return false;
			}
		}
		return true;

	}//end validateChannel()


	/**
	* Checks if the passed image information has all of the required tags or not and returns true if yes
	*
	* @param array	$image	image info that the user has created for this feed
	*
	* @return boolean
	* @access public
	*/
	function validateImage($image=Array())
	{
		if (empty($image)) {
			trigger_error(translate('missing_image_info'), E_USER_WARNING);
			return false;
		}
		$required = Array();
		$rss_type = $this->attr('rss_types');
		switch ($rss_type) {
			case 'atom_1.0':
				return true;
			break;
			default:
				$required = Array('title', 'url', 'link');
		}
		foreach ($required as $tag) {
			if (!in_array($tag, array_keys($image))) {
				trigger_error(translate('required_attribute').' "'.$tag.'" '.translate('missing_for_image'), E_USER_WARNING);
				return false;
			}
		}
		return true;

	}//end validateImage()


}//end class

?>
