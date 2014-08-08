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
* $Id: rss_feed_writer.inc,v 1.12 2012/08/30 00:56:51 ewang Exp $
*
*/


/**
* RSS_Feed_Writer
*
* Purpose
* Creates an XML document with all the data that is provided
*
* @author  Pratibha Gannavarapu <pgannavarapu@squiz.net>
* @version $Revision: 1.12 $
* @package Fudge
* @subpackage RSS_Feeds
*/
class RSS_Feed_Writer
{

	/**
	* The items are stored in this array which is an associative array
	*
	* @var array
	*/
	var $items = Array();

	/**
	* The image info is stored in here
	*
	* @var array
	*/
	var $image = Array();

	/**
	* The Channel info of the RSS is saved in this array
	*
	* @var array
	*/
	var $channel = Array();

	/**
	* The RSS string for all of the save information
	*
	* @var string
	*/
	var $rss = '';

	/**
	* The type of RSS that is being read eg.. RSS , ATOM etc
	*
	* @var string
	*/
	var $type = '';

	/**
	* The Version of the type of RSS eg.. could be RSS 1.0, RSS 2.0 etc
	*
	* @var string
	*/
	var $version = '';


	/**
	* Constructor
	*
	*/
	function RSS_Feed_Writer()
	{

	}//end constructor


	/**
	* Assigns the class variable items
	*
	* @param array	$items	the items to be added into the array
	*
	* @return void
	* @access public
	*/
	function addItems($items)
	{
		if (empty($items)) {
			return TRUE;
		}
		if ($this->_isTypeVersionSet()) {
			if (!is_array($items)) {
				trigger_error('You can assign Associative Arrays only to the items', E_USER_ERROR);
				return FALSE;
			}
			$this->items = $items;
			return TRUE;
		} else {
			return FALSE;
		}

	}//end addItems()


	/**
	* Adds the channel info into the channel array
	*
	* @param array	$channel	the channel to be added into the array
	*
	* @return void
	* @access public
	*/
	function addChannel($channel)
	{
		if (empty($channel)) {
			trigger_error('You cannot set empty values to channel', E_USER_ERROR);
			return FALSE;
		}
		if ($this->_isTypeVersionSet()) {
			if (!is_array($channel)) {
				trigger_error('You can assign Associative Arrays only to the items', E_USER_ERROR);
				return FALSE;
			}
			$this->channel = $channel;
			return TRUE;
		} else {
			return FALSE;
		}

	}//end addChannel()


	/**
	* Adds the image info into the image array
	*
	* @param array	$image	the image to be added into the array
	*
	* @return void
	* @access public
	*/
	function addImage($image)
	{
		if (empty($image)) {
			trigger_error('You cannot set empty values to image	', E_USER_ERROR);
			return FALSE;
		}
		if ($this->_isTypeVersionSet()) {
			if (!is_array($image)) {
				trigger_error('You can assign Associative Arrays only to the items', E_USER_ERROR);
				return FALSE;
			}
			$this->image = $image;
			return TRUE;
		} else {
			return FALSE;
		}

	}//end addImage()


	/**
	* Saves the type and version of the RSS to be generated
	*
	* @param string	$type		the type of RSS to be created
	* @param string	$version	the version of the RSS
	*
	* @return void
	* @access public
	*/
	function setTypeVersion($type='', $version='')
	{
		if (empty($type)  || empty($version)) {
			trigger_error('You Have To Set The Type And The Version Of RSS Before Adding the element', E_USER_WARNING);
			return FALSE;
		} else {
			if (!is_string($type) || !is_string($version)) {
				trigger_error('The Type And Version Of The RSS Feed To Be Generated Have To Be Strings', E_USER_WARNING);
				return FALSE;
			}
			$type_version = $type.'_'.$version;
			switch ($type_version) {
				case 'rss_1.0':
				break;
				case 'rss_2.0':
				break;
				case 'atom_1.0':
				break;
				default:
					trigger_error('The Type The RSS Feed Can Only Be Either rss Or atom', E_USER_ERROR);
				return FALSE;
			}
			$this->type = $type;
			$this->version = $version;
			return TRUE;
		}

	}//end setTypeVersion()


	/**
	* Gets the RSS in a string format for all of the information saved like the channel, image and items
	*
	* @param boolean	$header		if an appropriate header has to be included or not. Defaults to TRUE
	* @param string		$endoing	encoding attribute in XML declaration. Default is Western European(ISO)
	*
	* @return string
	* @access public
	*/
	function getRSSString($header=TRUE, $encoding='iso-8859-1')
	{
		$channel = $this->channel;
		$image = $this->image;
		$items = $this->items;
		$type = $this->type;
		$version = $this->version;
		// if no type or version is specified then trigger error
		// we NEED TYPE AND VERSION as this writer can build rss1.0, rss2.0, atom 1.0 versions
		if (empty($type) || empty($version)) {
			trigger_error('There Is No Type Or Version Set.. Please Set the Type And Version And Try Again', E_USER_WARNING);
			return;
		}
		// if the channel is empty ir the items is empoty then trigger error as there is no feed without items or channel
		if (empty($channel)) {
			trigger_error('Channel Info Required As Per The RSS Standards', E_USER_WARNING);
			return;
		}

		$name = 'item';
		ob_start();
		if ($header) header('Content-type: text/xml');
		echo '<?xml version="1.0" encoding="'.$encoding.'"?>';

		// open tag for atom is feed
		// for rss 1.0 it is rdf:RDF
		// for rss 2.0 it is rss version 2.0
		if (strcasecmp($type, 'atom') == 0) {
			$type = 'feed';
			$name = 'entry';
		}
		if (($type == 'rss') && ($version == '1.0')) {
			echo '<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/">';
		} else {
			echo '<'.$type;
			if ($type == 'feed') {
				echo ' xml:lang="en" xmlns="http://www.w3.org/2005/Atom"';
			} else {
				echo ' version="'.$version.'"';
			}
			echo '>';
		}
		// you do not have channel tag in atom 1.0 Instead we use feed tag itself
		if ($type != 'feed') {
			$type .= ' '.$version;

			if ($type == 'rss 2.0') {
				echo '<channel>';
			} else {
				if (isset($channel['rdf:about'])) {
					echo '<channel rdf:about="'.$channel['rdf:about'].'">';
				} else {
					trigger_error('A required attribute "rdf:about" is undefined for the Channel tag', E_USER_ERROR);
					ob_end_clean();
					return '';
				}
			}
		}
		// add data to the chaannel or the feed tags
		foreach ($channel as $key => $info) {
			if ($key == 'rdf:about') continue;
			if (is_array($info)) {
				if ($type != 'feed') {
					trigger_error('Attributes And SubTags In Channel Not Allowed For RSS Versions', E_USER_ERROR);
					ob_end_clean();
					return '';
				}
				$keys = array_keys($info);
				if (in_array('ATTRIBUTES', $keys)) {
					echo '<'.$key.' ';
					$attributes = $info['ATTRIBUTES'];
					foreach ($attributes as $sub => $sub_info) {
						// change the tags if any present in the info
						$sub_info = str_replace('&', '&amp;', $sub_info);
						$sub_info = str_replace('<', '&lt;', $sub_info);
						$sub_info = str_replace('>', '&gt;', $sub_info);
						echo $sub.'="'.$sub_info.'" ';
					}
					echo '>';
				}
				if (in_array('SUBTAGS', $keys)) {
					echo '<'.$key.'>';
					$sub_tags = $info['SUBTAGS'];
					foreach ($sub_tags as $sub => $sub_info) {
						// change the tags if any present in the info
						$sub_info = str_replace('&', '&amp;', $sub_info);
						$sub_info = str_replace('<', '&lt;', $sub_info);
						$sub_info = str_replace('>', '&gt;', $sub_info);
						echo '<'.$sub.'>'.$sub_info.'</'.$sub.'>';
					}
				}
				echo '</'.$key.'>';
			} else {
				// change the tags if any present in the info
				$info = str_replace('&', '&amp;', $info);
				$info = str_replace('<', '&lt;', $info);
				$info = str_replace('>', '&gt;', $info);
				echo '<'.$key.'>'.$info.'</'.$key.'>';
			}
		}//end foreach channnel

		if ($type == 'rss 1.0') {
			echo '<items><rdf:Seq>';
			foreach ($items as $item) {
				echo '<rdf:li rdf:resource="'.$item['rdf:about'].'"/>';
			}
			echo '</rdf:Seq></items></channel>';
		}

		if (!empty($image)) {
			if ($type == 'feed') {
				echo '<logo>';
			} else {
				echo '<image>';
			}
			foreach ($image as $key => $info) {
				// change the tags if any present in the info
				$info = str_replace('&', '&amp;', $info);
				$info = str_replace('<', '&lt;', $info);
				$info = str_replace('>', '&gt;', $info);
				if (is_array($info)) {
					echo '<'.$key.'>';
					foreach ($info as $sub => $sub_info) {
						echo '<'.$sub.'>'.$sub_info.'</'.$sub.'>';
					}
					echo '</'.$key.'>';
				} else {
					echo '<'.$key.'>'.$info.'</'.$key.'>';
				}
			}
			if ($type == 'feed') {
				echo '</logo>';
			} else {
				echo '</image>';
			}
		}

			foreach ($items as $item) {
				if ($type != 'rss 1.0') {
					echo '<'.$name.'>';
				} else {
					echo '<item rdf:about="'.$item['rdf:about'].'">';
					unset($item['rdf:about']);
				}
				foreach ($item as $key => $info) {
					if (is_array($info)) {
						$keys = array_keys($info);
						if (in_array('ATTRIBUTES', $keys)) {
							echo '<'.$key.' ';
							$attributes = $info['ATTRIBUTES'];
							foreach ($attributes as $sub => $sub_info) {
								// change the tags if any present in the info
								$sub_info = str_replace('&', '&amp;', $sub_info);
								$sub_info = str_replace('<', '&lt;', $sub_info);
								$sub_info = str_replace('>', '&gt;', $sub_info);
								echo $sub.'="'.$sub_info.'" ';
							}
							echo '>';
						}
						if (in_array('SUBTAGS', $keys)) {
							echo '<'.$key.'>';
							$sub_tags = $info['SUBTAGS'];
							foreach ($sub_tags as $sub => $sub_info) {
								// change the tags if any present in the info
								$sub_info = str_replace('&', '&amp;', $sub_info);
								$sub_info = str_replace('<', '&lt;', $sub_info);
								$sub_info = str_replace('>', '&gt;', $sub_info);
								echo '<'.$sub.'>'.$sub_info.'</'.$sub.'>';
							}
						}
						echo '</'.$key.'>';
					} else {
						// change the tags if any present in the info
						$info = str_replace('&', '&amp;', $info);
						$info = str_replace('<', '&lt;', $info);
						$info = str_replace('>', '&gt;', $info);
						echo '<'.$key.'>'.$info.'</'.$key.'>';
					}
				}//end foreach item
				echo '</'.$name.'>';
			}//end foreach items

		if ($type != 'rss 1.0') {
			if ($type != 'feed') {
				// rss 2.0
				echo '</channel>';
				echo '</'.$this->type.'>';
			} else {
				// atom 1.0
				echo '</feed>';
			}
		} else {
			// rss 1.0
			echo '</rdf:RDF>';
		}

		$rss = ob_get_contents();
		ob_end_clean();
		return $rss;

	}//end getRSSString()


//--        PRIVATE OR INTERNAL FUNCTIONS        --//


	/**
	* Checks if the type and version are set or not and returns TRUE if set else triggers error
	*
	* @return boolean
	* @access private
	*/
	function _isTypeVersionSet()
	{
		if (!empty($this->type) && (!empty($this->version))) {
			return TRUE;
		} else {
			trigger_error('You Have To Set The Type And The Version Of RSS Before Adding the element', E_USER_WARNING);
			return FALSE;
		}

	}//end _isTypeVersionSet()


}//end class


?>
