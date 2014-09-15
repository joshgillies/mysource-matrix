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
* $Id: page_rss_feed.inc,v 1.30 2012/08/30 00:59:01 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_RSS_Feed
*
* Purpose
*
*
* @author  Pratibha Gannavarapu <pgannavarapu@squiz.net>
* @version $Revision: 1.30 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Page_RSS_Feed extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

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
		$links[SQ_LINK_NOTICE]['listing_engine'] = Array('card' => 'M', 'exclusive' => FALSE);
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
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
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
		$dummy_asset = new $type();
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
	* This function looks on converting the channel, image and items arrays into the RSS Feed Writer class acceptable forms
	* adds them to the writer and prints the feed. The Writer class acceptable form of the channel and items arrays is as follows:
	*	Array(
	*		tag1 => value,
	*		tag2 => Array (
	*					'SUBTAGS' => Array(
	*									subtag1	=> value,
	*									subtag2	=> value,
	*								),
	*				),
	*		tag3 => Array (
	*					'ATTRIBUTES' => Array(
	*										attr1 => value,
	*										attr2 => value,
	*									),
	*				),
	* 	)
	* This is explained more clearly with an example in the _traverseChannel function
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		// check http auth if configured to do so
		if ($this->attr('http_auth') && isset($_SERVER['PHP_AUTH_USER'])) {
			// spoof normal login
			$_REQUEST['SQ_ACTION'] = 'login';

			$_POST['SQ_LOGIN_USERNAME'] = $_SERVER['PHP_AUTH_USER'];
			$_POST['SQ_LOGIN_PASSWORD'] = $_SERVER['PHP_AUTH_PW'];

			$_POST['SQ_LOGIN_KEY'] = $GLOBALS['SQ_SYSTEM']->loginKey();
			$GLOBALS['SQ_SYSTEM']->_processGlobalActions();
		}


		// check if read access
		if (!parent::readAccess()) {

			ob_start();
				$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), sprintf(translate('cannot_access_asset'), $this->name));
			$login = ob_get_clean();

			// handle http authentication
			if ($this->attr('http_auth')) {
				header('WWW-Authenticate: Basic realm="'.$this->name.'"');
				header('HTTP/1.0 401 Unauthorized');
			}
			echo $login;

			return;
		}
		// try to get the feed from cache
		$rss = '';
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$rss = $cm->loadFromCache($this->id, $this->type(), 'ctx'.$contextid);

		if ($this->charset) {
			$charset = $this->charset;
		} else {
			$charset = SQ_CONF_DEFAULT_CHARACTER_SET;
		}

		// if cache returns nothing then build the feed
		if ($rss !== FALSE) {
			$rss = unserialize($rss);
		} else {
			// get the assets to be listed in the feed
			$num_assets_as_items = $this->attr('num_assets_as_items');
			$assets_to_list = Array();
			$rss_source_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'listing_engine', FALSE, 'major', 'generate_rss');

			if (empty($rss_source_links)) {
				trigger_error(translate('no_listing_engine_assets_linked'), E_USER_WARNING);
				return;
			}
			foreach ($rss_source_links as $links) {
				$listing_engine = $GLOBALS['SQ_SYSTEM']->am->getAsset($links['minorid']);
				$list_of_assets = $listing_engine->getAssetList();
				$listing_engine->filterAssetList($list_of_assets);
				$assets_to_list += $list_of_assets;
			}

			$channel = $this->attr('channel');
			$image = $this->attr('image');
			$items = $this->attr('items');

			// A feed should consist of both channel and items. If either of them is empty then throw an error
			// If there are no assets to list then generate a valid empty feed
			if (empty($channel) || empty($items)) {
				trigger_error(translate('no_channel_or_items_found'), E_USER_WARNING);
				return;
			} else {

				require_once SQ_FUDGE_PATH.'/rss_feeds/rss_feed_writer.inc';
				// Create an RSS Feed writer object and set the type and version of the feed to be built
				$writer = new RSS_Feed_Writer();
				$type = $this->attr('rss_types');
				$type = explode('_', $type);
				$writer->setTypeVersion($type[0], $type[1]);

				$new_channel = Array();
				$rss_type = implode('_', $type);

				// pass the user entered channel array to the _traverseChannel which now converts it into
				// RSS Feed Writer Class acceptable format
				$new_channel = $this->_traverseChannel($channel, $rss_type);

				// check if the user has entered all of the required tags for the channel information
				$no_errors = $this->validateChannel($new_channel);
				if (!$no_errors) return;

				// adding channel info to the writer
				$writer->addChannel($new_channel);

				// traverse the image information array in the same way as we did for the channel
				$new_image = $this->_traverseImage($image);

				if (!empty($new_image)) {
					// check if the user has enetered the values for all of the required tags
					$no_errors = $this->validateImage($new_image);
					if (!$no_errors) return;
					// adding image details to the writer
					$writer->addImage($new_image);
				}

				// now we have to add items
				// Check if the user has eneterd the number of assets that are to be added to the list. If not
				// then we have to add all the assets to the feed.
				$new_items = Array();
				$index = 0;
				$mm = null;
				foreach ($assets_to_list as $assetid => $asset_type) {
					$asset_type = $asset_type[0]['type_code'];
					foreach ($items as $type => $keywords) {
						if (!empty($num_assets_as_items)) {
							if ($index >= $num_assets_as_items) break;
						}

						if (strcasecmp($type, $asset_type) == 0) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							if (!is_null($asset) && ($asset instanceof $asset_type)) {
								foreach ($keywords as $tag => $keyword) {
									// if the user enters multiple keywords or some text along with keywords then
									// we need to pick the keywords and insert the replacements in the entered text.
									if (strpos($keyword, '%') !== FALSE) {
										$sub_keywords = Array();
										require_once SQ_FUDGE_PATH.'/general/text.inc';
										$sub_keywords = retrieve_keywords_replacements($keyword);
										$replacements = Array();
										foreach ($sub_keywords as $key) {
											$replacements[$key] = $asset->getKeywordReplacement($key);
										}
										$replacement = replace_keywords($keyword, $replacements);
									} else {
										$replacement = $asset->getKeywordReplacement($keyword);
									}
									if (substr($keyword, 0, 15) == 'asset_metadata_') {
										$metadata_keyword = substr($keyword, 15);
										if (is_null($mm)) {
											$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
										}
										// get all the metadata keywords for this asset
										$metadata_values = $mm->getMetadataFieldValues($assetid, Array($metadata_keyword));

										foreach ($metadata_values as $field => $value) {
											$replacement = $value;
										}
									}
									if (strpos($tag, '_') !== FALSE) {
										$sub_tag = explode('_', $tag);
										$new_items[$index][$sub_tag[0]]['SUBTAGS'][$sub_tag[1]] = $replacement;
									} else if (strpos($tag, '.') !== FALSE) {
										$sub_tag = explode('.', $tag);
										$new_items[$index][$sub_tag[0]]['ATTRIBUTES'][$sub_tag[1]] = $replacement;
									} else {
										$new_items[$index][$tag] = $replacement;
									}
								}//end foreach keywords
							}//end if asset exists
							$index++;
						}//end if type and asset_type are equal
					}//end foreach items
				}//end foreach assets_to_list

				$no_errors = $this->validateItems($new_items);
				if (!$no_errors) return;
				$writer->addItems($new_items);
				$rss = $writer->getRSSString(FALSE, $charset);
				$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
				$r = $cm->saveToCache($this->id, $this->type(), 'ctx'.$contextid, serialize($rss));
			}
		}

		header('Content-type: text/xml; charset='.$charset);

		echo $rss;

	}//end printFrontend()


	/**
	* Traverses the entered channel array and returns it in the RSS Writer class acceptable format
	*
	* @param array	$channel	the channel information for this feed
	* @param string $rss_type	the rss type and version e.g. rss_1.0, atom_1.0
	*
	* @return array
	* @access private
	*/
	function _traverseChannel($channel, $rss_type)
	{
		// parse the entered channel information and build an array of information in the proper format
		// For example the channel array now is of the format:
		/*
			Array(
				title => value,
				description => value,
				author_name => value,
				author_email => value,
				link.href => assetid of the asset whose URL has to be picked up for the value,
				category.type => value,
			);
		*/
		// The above array now gets traversed using the for loop below resulting in an array of the following format
		/*
			Array(
				tag1 => value,
				tag2 => Array (
						 'SUBTAGS' => Array(
										subtag1	=> value,
										subtag2	=> value,
									  ),
						),
				tag3 => Array (
							'ATTRIBUTES' => Array(
												attr1 => value,
												attr2 => value,
											)
						)
			)
		*/
		// The example array should get transformed as follows:
		/*
			Array(
				title => value,
				description => value,
				author => Array(
							'SUBTAGS' => Array(
											name => value,
											email => value,
										),
						 ),
				link => Array(
							'ATTRIBUTES' => Array(
												href => asset url,
											),
						)
				category => Array(
								'ATTRIBUTES' => Array(
													type => value,
												),
							)
			)
		*/
		$new_channel = Array();
		foreach ($channel as $tag => $value) {
			if (strpos($tag, '_') !== FALSE) {
				$sub_tag = explode('_', $tag);
				if ($sub_tag[0] == 'link.href') { // case link.href_text
					$link_href_tag = explode('.', $sub_tag[0]);
					$new_channel[$link_href_tag[0]]['ATTRIBUTES'][$link_href_tag[1]] = $value;
				} else {
					$new_channel[$sub_tag[0]]['SUBTAGS'][$sub_tag[1]] = $value;
				}
			} else if (strpos($tag, '.') !== FALSE) {
				if ($tag == 'link.href') {
					if (assert_valid_assetid($value, '', TRUE, FALSE)) {
						$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
					}
				}
				$sub_tag = explode('.', $tag);
				$new_channel[$sub_tag[0]]['ATTRIBUTES'][$sub_tag[1]] = $value;
			} else {
				// converting date values into the respective formats
				if (in_array($tag, Array('updated', 'pubDate', 'lastBuildDate', 'published'))) {
					$timestamp = strtotime($value);
					if (($timestamp == -1) ||($timestamp === FALSE)) {
						$timestamp = mktime($value);
					}
					if ($rss_type == 'rss_2.0') {
						$value = date('D, j M Y H:i:s O', $timestamp);
					} else if ($rss_type == 'atom_1.0') {
						$value = date('Y-m-d', $timestamp).'T'.date('H:i:s', $timestamp).'Z';
					}
				} else {
					if ($tag == 'link') {
						if (assert_valid_assetid($value, '', TRUE, FALSE)) {
							$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
						}
					}
				}
				$new_channel[$tag] = $value;
			}
		}//end foreach of channel
		return $new_channel;

	}//end _traverseChannel()


	/**
	* Traverses the entered image array and returns it in the RSS Writer class acceptable format
	*
	* @param array	$image	the image information for this feed
	*
	* @return array
	* @access private
	*/
	function _traverseImage($image)
	{
		$new_image = Array();
		foreach ($image as $tag => $value) {
			if (strpos($tag, '_') !== FALSE) {
				$sub_tag = explode('_', $tag);
				$new_image[$sub_tag[0]][$sub_tag[1]] = $value;
			} else {
				if ($tag == 'link') {
					if (assert_valid_assetid($value, '', TRUE, FALSE)) {
						$value = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($value);
					}
				}
				$new_image[$tag] = $value;
			}
		} //end foreach where image array is traversed
		return $new_image;

	}//end _traverseImage()


	/**
	* Checks if the passed items have all of the required tags or not and returns TRUE if yes
	*
	* @param array	$items	items that the user has created for this feed
	*
	* @return boolean
	* @access public
	*/
	function validateItems($items=Array())
	{
		if (empty($items)) {
			return TRUE;
		} else {
			$required = Array();
			$rss_type = $this->attr('rss_types');
			switch ($rss_type) {
				case 'atom_1.0':
					$required = Array('id', 'updated', 'title', 'author');
				break;
				case 'rss_1.0':
					$required = Array('link', 'title', 'rdf:about');
				break;
				case 'rss_2.0':
					$required = Array('link', 'title', 'description');
				break;
			}
			foreach ($items as $item) {
				$keys = array_keys($item);
				if (in_array('category', $keys)) {
					$temp = $items['category']['ATTRIBUTUES'];
					if (!in_array('term', array_keys($temp))) {
						trigger_error(translate('required_attr_term_unassigned'), E_USER_WARNING);
						return FALSE;
					}
				}
				if ($rss_type == 'rss_2.0') {
					if (!in_array('title', $keys)) {
						if (!in_array('description', $keys)) {
							trigger_error(translate('required_tag_title_or_description_missing'), E_USER_WARNING);
							return FALSE;
						}
					}
					continue;
				}
				foreach ($required as $req) {
					if (!in_array($req, $keys)) {
						trigger_error('Required Tag '.$req.' Is Not Assigned In Items', E_USER_WARNING);
						return FALSE;
					}
				}
			}
			return TRUE;
		}

	}//end validateItems()


	/**
	* Checks if the passed channel information has all of the required tags or not and returns TRUE if yes
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
			return FALSE;
		}
		$required = Array();
		$rss_type = $this->attr('rss_types');
		switch ($rss_type) {
			case 'rss_1.0':
				$required = Array('title', 'link', 'description', 'rdf:about');
			break;
			case 'rss_2.0':
				$required = Array('title', 'link', 'description');
			break;
			case 'atom_1.0':
				$required = Array('title', 'id', 'updated');
		}
		foreach ($required as $tag) {
			if (!in_array($tag, array_keys($channel))) {
				trigger_error(translate('required_attribute').' "'.$tag.'" '.translate('missing_for_channel'), E_USER_WARNING);
				return FALSE;
			}
		}
		return TRUE;

	}//end validateChannel()


	/**
	* Checks if the passed image information has all of the required tags or not and returns TRUE if yes
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
			return FALSE;
		}
		$required = Array();
		$rss_type = $this->attr('rss_types');
		switch ($rss_type) {
			case 'atom_1.0':
				return TRUE;
			break;
			default:
				$required = Array('title', 'url', 'link');
		}
		foreach ($required as $tag) {
			if (!in_array($tag, array_keys($image))) {
				trigger_error(translate('required_attribute').' "'.$tag.'" '.translate('missing_for_image'), E_USER_WARNING);
				return FALSE;
			}
		}
		return TRUE;

	}//end validateImage()


	/**
	* Returns TRUE if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		if (SQ_IN_LIMBO || SQ_IN_BACKEND) {
			return parent::readAccess($assetids);
		}
		return TRUE;

	}//end readAccess()


}//end class

?>
