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
* $Id: data_source_rss.inc,v 1.17.6.1 2013/09/30 05:50:44 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Data_Source_RSS
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.17.6.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_RSS extends Data_Source
{
	var $reader;


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor

	
	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()
	
	
	/**
	* Returns an array of the result of xml parsing either from cache or by parsing the file again
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);
		// use feed url as the cache key
		$url = $this->attr('url');

		$result = Array();
		$result = parent::getResultSet($url);
		if (empty($result)) {
			if (!empty($url)) {
				if (isset($this->_tmp[$url])) {
					$result = $this->_tmp[$url];
				}
				if (empty($result)) $result = $this->getItems();
				if (!empty($result)) {
					parent::setResultSet($result, $url);
					$this->_tmp[$url] = $result;
				}
			}
		}
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
		return $result;

	}//end getResultSet()


	/**
	* Returns an array of the result by parsing the file
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		$new_result = Array();
		$result = $this->parseXML();
		if (!$result) return Array();

		$feed_reader = $this->reader;
		$result = $feed_reader->getItems();
		$type = $feed_reader->getRssType();
		foreach ($result as $index => $info) {
			foreach ($info as $tag => $val_array) {
				// for atom 1.30 type of rss feeds we need to add default keywords of
				// title, description and link
				if ($type == 'atom_1.0') {
					if (isset($val_array['value'])) {
						switch ($tag) {
							case 'content':
								$new_result[$index]['description'] = $val_array['value'];
							break;
							case 'heading':
								$new_result[$index]['title'] = $val_array['value'];
							break;
						}
					}
				}
				if ($tag != 'link') {
					if (isset($val_array['value'])) {
						$new_result[$index][$tag] = $val_array['value'];
					}
				} else {
					if ($type == 'atom_1.0') {
						$new_result[$index][$tag] = $val_array['_attributes']['HREF'];
					}
					if (isset($val_array['_attributes'])) {
						$attributes = $val_array['_attributes'];
						foreach ($attributes as $attr => $value) {
							$new_result[$index][$tag.'_'.strtolower($attr)] = $value;
						}
					} else {
						$new_result[$index][$tag] = $val_array['value'];
					}
				}
			}//end foreach

			// timestamp data source record set attribute
			$date_f = substr($this->attr('date_field'), strlen('ds__'));
			if (!empty($date_f)) {
				if (isset($info[$date_f])) {
					$date_string = $info[$date_f]['value'];
					if ($type == 'rss_2.0') {
						// rss 2.0 uses GMT date format
						$timestamp = strtotime($date_string);
					} else {
						// atom 1.0 uses the date construct
						$timestamp = iso8601_ts($date_string);
					}
					if ($timestamp) {
						$new_result[$index]['timestamp'] = $timestamp;
					}
				}
			}
		}//end foreach

		return $new_result;

	}//end getItems()


	/**
	* Return the keywords for this asset type.
	* This function is called in asset-listing situations like the asset listing, search page etc
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();
		$keywords['data_source_rss_channelInfo'] = translate('rss_channel_info');
		$keywords['data_source_rss_imageInfo'] = translate('rss_image_info');
		$keywords['data_source_rss_rss_type'] = translate('rss_type');
		$keywords['data_source_rss_textinput'] = translate('rss_text_input');
		$keywords['data_source_rss_items'] = translate('rss_items');
		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Used for parsing the XML file
	*
	* @return void
	* @access public
	*/
	function parseXML()
	{
		$url = $this->attr('url');
		require_once SQ_FUDGE_PATH.'/rss_feeds/rss_feed_reader.inc';
		$feed_reader = new RSS_Feed_Reader();
		$feed_reader->delimiter = $this->attr('data_delimiter');
		$this->reader = $feed_reader;
		
		if (!empty($url)) {
			if ($feed_reader->setInputFile($url)) {
				$result = $feed_reader->parse();
				return $result;
			} else {
				return FALSE;
			}
		} else {
			return FALSE;
		}

	}//end parseXML()


	/**
	* Get bodycopy keyword replacement
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$replacement = NULL;
		if (strpos( $keyword, 'data_source_rss') === FALSE) {
			$replacement = parent::getKeywordReplacement($keyword);
		} else {
			$keyword = substr($keyword, strlen('data_source_rss_'));
			if (!empty($keyword)) {
				$res = $this->parseXML();
				$reader = $this->reader;
				$function = 'get'.ucwords($keyword);
				$result = $reader->{$function}();
			}
			if (is_array($result)) {
				foreach ($result as $index => $info) {
					foreach ($info as $tag => $val_array) {
						if ($tag != 'link') {
							if (isset($val_array['value'])) {
								$replacement[$index][$tag] = $val_array['value'];
							}
						} else {
							if (isset($val_array['_attributes'])) {
								$attributes = $val_array['_attributes'];
								foreach ($attributes as $attr => $value) {
									$replacement[$index][$tag.'_'.strtolower($attr)] = $value;
								}
							} else {
								$replacement[$index][$tag] = $val_array['value'];
							}
						}
					}
				}
				ob_start();
				?>
				<table>
				<?php
				foreach ($replacement as $replace) {
					foreach ($replace as $key => $value) {
						?>
						<tr>
							<td><?php echo $key;?></td>
							<td><?php echo $value;?></td>
						</tr>
						<?php
					}
				}
				?>
				</table>
				<?php
				$replacement = ob_get_contents();
				ob_end_clean();
			} else {
				$replacement = $result;
			}
		}//end else

		return $replacement;

	}//end getKeywordReplacement()


}//end class

?>
