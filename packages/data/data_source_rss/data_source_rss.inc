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
* $Id: data_source_rss.inc,v 1.7 2006/03/29 00:51:20 pgannavarapu Exp $
*
*/



/**
* Data_Source_RSS
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
	require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
	require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
	require_once SQ_LIB_PATH.'/html_form/html_form.inc';

class Data_Source_RSS extends Data_Source
{
	var $reader;


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function Data_Source_RSS($assetid=0)
	{
		$this->Data_Source($assetid);

	}//end constructor


	/**
	* Returns an array of the result of xml parsing either from cache or by parsing the file again
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
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
		$feed_reader =& $this->reader;
		$result = $feed_reader->getItems();
		$type = $feed_reader->getRssType();
		foreach ($result as $index => $info) {
			foreach ($info as $tag => $val_array) {
				//for atom 1.30 type of rss feeds we need to add default keywords of
				//title, description and link
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
			}
		}
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
		$feed_reader =& new RSS_Feed_Reader();
		$this->reader =& $feed_reader;
		if (!empty($url)) {
			$feed_reader->setInputFile($url);
			$result = $feed_reader->parse();
			return $result;
		} else {
			return false;
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
		$replacement = null;
		if (strpos( $keyword, 'data_source_rss') === false) {
			$replacement = parent::getKeywordReplacement($keyword);
		} else {
			$keyword = substr($keyword, strlen('data_source_rss_'));
			if (!empty($keyword)) {
				$res = $this->parseXML();
				$reader =& $this->reader;
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
		}
		return $replacement;

	}//end getKeywordReplacement()


}//end class

?>
