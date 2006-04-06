<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: report_links_edit_fns.inc,v 1.14 2006/04/06 06:42:05 arailean Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';

/**
* Report_Links_Edit_Fns
*
* Purpose
*	Generate a report of links
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Report_Links_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Report_Links_Edit_Fns()
	{
		$this->Report_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = '0';

	}//end constructor


	/**
	* Generate this report until complete
	*
	* @param object	&$job		a reference to the array of information about
	* @param array	&$step_data	a reference to the array of information about
	*							the current step of the HIPO job that is running this report
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$owner =& $GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['assetid'], 'report_links');

		if (!isset($job->_running_vars['results'])) {
			$job->_running_vars['todo'] = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->getRootAssetid($owner), 'content_type', FALSE);
			$job->_running_vars['done'] = Array();
			$job->_running_vars['results'] = Array();

			if (!is_dir($owner->data_path)) {
				if (!create_directory($owner->data_path)) {
					trigger_localised_error('CMS0005', E_USER_WARNING);
					return FALSE;
				}
			}
			$temp_file = fopen($owner->data_path.'/report.tmp', 'w');
			if ($temp_file === FALSE) {
				trigger_localised_error('CMS0006', E_USER_WARNING);
				return FALSE;
			}
			fwrite($temp_file, "<links>\n");
		} else {
			$temp_file = fopen($owner->data_path.'/report.tmp', 'a');
			if ($temp_file === FALSE) {
				trigger_localised_error('CMS0006', E_USER_WARNING);
				return FALSE;
			}
		}

		if (!empty($job->_running_vars['todo'])) {

			$assetid = array_shift(array_keys($job->_running_vars['todo']));
			$asset_type = $job->_running_vars['todo'][$assetid];
			unset($job->_running_vars['todo'][$assetid]);
			if ($asset_type == 'content_type_nest_content') {
				$step_data['message'] = translate('cms_report_links_skipping_nest_content', $assetid);
			} else {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

				if (!is_null($asset)) {
					$step_data['message'] = translate('cms_report_links_generating');

					$content = '';
					$wysiwyg_html = $asset->attr('html');

					$master_matches = Array();
					preg_match_all('%(href|src)=[\'"]([^\'\s"]*)%', $wysiwyg_html, $master_matches);
					$matches = $master_matches[2];

					foreach ($matches as $match) {
						// skip keywords
						$keywords = retrieve_keywords_replacements($match);
						if (count($keywords) == 1 && $match == '%'.$keywords[0].'%') {
							continue;
						}

						if (!$match) continue;
						$link_valid = '0';
						$link_description = '';
						$link_type = '';

						$results = Array();
						preg_match('|^\./\?a=([0-9]+)#?([^$]*)?|',$match, $results);
						$validated_results = FALSE;
						if (!empty($results)) {
							$link_type = 'internal';
							$validated_results = $this->validateInternalLink($results);
						} else {
							if (strpos(strtolower($match), 'mailto') === 0) {
								$link_type = 'mailto';
								$match = preg_replace('/mailto:/i','',$match);
								list($match) = explode('?',$match);
								$match = trim($match);
								$validated_results = $this->validateEmailLink($match);
							} else {
								$link_type = 'external';
								$validated_results = $this->validateExternalLink($match);
							}
						}

						if ($validated_results === FALSE) continue;

						$content .= "\t\t".'<link type="'.$link_type.'" valid="'.$validated_results['valid'].'">'."\n";
						$content .= "\t\t\t".'<html><![CDATA['.$match.']]></html>'."\n";
						$content .= "\t\t\t".'<description><![CDATA['.$validated_results['description'].']]></description>'."\n";
						$content .= "\t\t".'</link>'."\n";
					}//end foreach

					if (!empty($content)) {
						$content = "\t".'<asset assetid="'.$asset->id.'" name="'.htmlSpecialChars($asset->name).'" short_name="'.htmlSpecialChars($asset->short_name).'" type_code="'.$asset->type().'" status="'.$asset->status.'">'."\n".$content."\t".'</asset>'."\n";
						fwrite($temp_file, $content);
					}

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);

				} else {
					$step_data['message'] = translate('cms_report_links_skipping_asset', $assetid);
					$job->_addError(translate('cms_report_links_asset_not_exist', $assetid));
				}

			}//end else nest content cell

			// add this assetid to the done array so we dont do it again
			$job->_running_vars['done'][] = $assetid;
		}//end if

		if (empty($job->_running_vars['todo'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;

			// move the temp file to become the real report XML file
			fwrite($temp_file, '</links>');
			if (!copy($owner->data_path.'/report.tmp', $owner->data_path.'/report.xml')) {
				trigger_localised_error('CMS0007', E_USER_WARNING);
				return FALSE;
			}
			if (!unlink($owner->data_path.'/report.tmp')) {
				trigger_localised_error('CMS0008', E_USER_WARNING);
			}
		} else {
			$total = count($job->_running_vars['todo']) + count($job->_running_vars['done']);
			$step_data['percent_done'] = (count($job->_running_vars['done']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		fclose($temp_file);
		return TRUE;

	}//end generateReport()


	/**
	* Check the validity of an internal link
	*
	* @param array	$link	the internal link in the format:
	*						Array
	*						(
	*						[0] => ./?a=123#anchor_name
	*						[1] => 142
	*						[2] => anchor_name
	*						)
	*
	* @return array
	* @access public
	*/
	function validateInternalLink($link)
	{
		$asset = NULL;
		$assetid = $link[1];
		if ($assetid) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
		}

		if (is_null($asset)) {
			// the asset was invalid
			$description = translate('cms_report_links_point_invalid_assetid');
			$valid = '0';
		} else if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($assetid, TRUE)) {
			// the asset is in the trash
			$description = translate('cms_report_links_point_to_trash');
			$valid = '0';
		} else if (!empty($link[2])) {
			$contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($assetid);
			$anchorExists = FALSE;

			// check if we are looking for an interface for this asset
			if (preg_match('/\/_admin|\/_edit/', $link[2], $matches)) {
				$anchorExists = TRUE;
			} else if (preg_match('/^\?(.+=.*)*/', $link[2], $matches)) {
				$anchorExists = TRUE;
			} else {
			// otherwise lets traul the contents for a matching anchor
				foreach ($contents as $content) {
					if (preg_match('/<a name="'.$link[2].'">/', $content, $matches)) {
						$anchorExists = TRUE;
						break;
					}
				}
			}

			if (!$anchorExists) {
				$valid = '0';
				$description = translate('cms_report_links_anchor_not_exist', $link[2]);
			} else {
				$valid = '1';
				$description = '';
			}
		} else {
			$valid = '1';
			$description = '';
		}
		if (!is_null($asset)) {
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		}

		return Array('valid' => $valid, 'description' => $description);

	}//end validateInternalLink()


	/**
	* Check the validity of an external link
	*
	* @param string	$url	the url being validated
	*
	* @return array
	* @access public
	*/
	function validateExternalLink($url)
	{
		// ignore bookmarks
		if (substr($url, 0, 1) == '#') return FALSE;

		$url_parts = @parse_url($url);
		if (empty($url_parts['scheme'])) {
			$description = translate('cms_report_links_missing_protocol');
			$valid = '0';
		} else if (empty($url_parts['host'])) {
			$description = translate('cms_report_links_missing_hostname');
			$valid = '0';
		} else if ($url_parts['scheme'] != 'http') {
			$description = translate('cms_report_links_cannot_test_protocol');
			$valid = '1';
		} else {
			if (empty($url_parts['path'])) {
				$url_parts['path'] = '/';
			}
			if (empty($url_parts['port'])) {
				$url_parts['port'] = '80';
			}

			$url_path = $url_parts['path'];
			if (!empty($url_parts['query'])) {
				$url_path .= '?'.$url_parts['query'];
			}

			$socket = @fsockopen($url_parts['host'], $url_parts['port'], $errno, $errstr, 15);
			if (!$socket) {
				$description = translate('cms_report_links_invalid_url');
				$valid = '1';
			} else {
				fwrite ($socket, 'HEAD '.$url_path." HTTP/1.0\r\nHost: ".$url_parts['host']."\r\n\r\n");
				stream_set_timeout($socket, 5);
				$response = fread($socket, 128);
				$headers = fread($socket, 2000);
				$info = stream_get_meta_data($socket);

				if (preg_match('|200 ok|i', $response)) {
					$description = '';
					$valid = '1';
				} else if (preg_match('|302 found|i', $response)) {
					$description = '';
					$valid = '1';
				} else if (preg_match('|304 not modified|i', $response)) {
					$description = '';
					$valid = '1';
				} else if (preg_match('|404 not found|i', $response)) {
					$description = translate('cms_report_links_404');
					$valid = '0';
				} else if (preg_match('|301 moved|i', $response)) {
					$valid = '1';
					preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
					if (isset($matches[1])) {
						$description = translate('cms_report_links_301_with_new_link', $matches[1]);
					} else {
						$description = translate('cms_report_links_301');
					}
				} else if (preg_match('|302 moved|i', $response)) {
					// should this
					$valid = '1';
					preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
					if (isset($matches[1])) {
						$description = translate('cms_report_links_302_with_new_link', $matches[1]);
					} else {
						$description = translate('cms_report_links_302');
					}
				} else if ($info['timed_out']) {
					$description = translate('cms_report_links_server_timeout');
					$valid = '0';
				} else {
					$description = translate('cms_report_links_unknown_response', $response);
					$valid = '0';
				}
				fclose($socket);
			}//end else
		}//end else

		return Array('valid' => $valid, 'description' => $description);

	}//end validateExternalLink()


	/**
	* Check the validity of an mailto
	*
	* @param string	$mailto	the mailto being validated
	*
	* @return array
	* @access public
	*/
	function validateEmailLink($mailto)
	{
		require_once SQ_FUDGE_PATH.'/general/www.inc';
		if (!valid_email($mailto)) {
			$description = translate('cms_report_links_invalid_email');
			$valid = '0';
		} else {
			$description = '';
			$valid = '1';
		}

		return Array('valid' => $valid, 'description' => $description);

	}//end validateEmailLink()


	/**
	* Paint the interface for viewing a report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReport(&$asset, &$o, $prefix)
	{
		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		require_once SQ_FUDGE_PATH.'/general/general.inc';

		$report_path = $asset->data_path.'/report.xml';
		if (!is_file($report_path)) {
			echo translate('report_not_generated');
			return;
		}

		$input =& new XML_Tree($report_path);
		$root  =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_localised_error('CMS0009', E_USER_WARNING, $report_path, $root->getMessage(), $root->getUserInfo());
			return;
		}

		$reported_links = Array(
							'internal'	=> Array(),
							'external'	=> Array(),
							'mailto'	=> Array(),
						  );

		$display_options = $asset->attr('display_options');

		foreach ($root->children as $asset_element) {
			if ($asset_element->name != 'asset') continue;
			foreach ($asset_element->children as $link) {
				if ($link->name != 'link') continue;
				assert_isset_array_index($reported_links, $link->attributes['type'], 'Unknown link type "'.$link->attributes['type'].'"');

				$new_link = Array();
				foreach ($link->children as $link_child) {
					switch ($link_child->name) {
						case 'html' :
							$new_link['html'] = $link_child->content;

							// get the assetid from the html link
							if ($link->attributes['type'] == 'internal') {
								$to_assetid = preg_replace('|\./\?a=([0-9]+).*|', '\\1', $link_child->content);
								$new_link['data'] = $to_assetid;
							} else {
								$new_link['data'] = $link_child->content;
							}
						break;
						case 'description' :
							$new_link['description'] = $link_child->content;
						break;
					}
				}//end foreach

				$new_link['valid'] = $link->attributes['valid'];

				$display_link = TRUE;
				switch ($link->attributes['type']) {
					case 'internal':
						if ($link->attributes['valid']) {
							if (!$display_options['internal_working']) {
								$display_link = FALSE;
							}
						} else {
							if (!$display_options['internal_broken']) {
								$display_link = FALSE;
							}
						}
					break;
					case 'external':
						if ($link->attributes['valid']) {
							if (!$display_options['external_working']) {
								$display_link = FALSE;
							}
						} else {
							if (!$display_options['external_broken']) {
								$display_link = FALSE;
							}
						}
					break;
					case 'mailto':
						if ($link->attributes['valid']) {
							if (!$display_options['email_working']) {
								$display_link = FALSE;
							}
						} else {
							if (!$display_options['email_broken']) {
								$display_link = FALSE;
							}
						}
					break;
					default:
					break;
				}//end switch
				if ($display_link) {
					$reported_links[$link->attributes['type']][$asset_element->attributes['assetid']][] = $new_link;
				}
			}//end foreach
		}//end foreach

		// TODO: this translation has a 'note' prefix that is supposed to look like
		//      a field header... could this be set up as a field?
		echo translate('cms_report_links_current');
		$o->closeSection();

		if ($display_options['internal_section'] == TRUE) {
			$o->openSection('Internal Links');
				$o->openField('');
					if (empty($reported_links['internal'])) {
						echo 'No internal links were found';
					} else {
						?>
						<table class="sq-backend-table" style="border: 1px solid #DDDDDD;" border="0">
							<tr>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('found_in') ?></td>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('linking_to') ?></td>
								<th class="sq-backend-table-header"><?php echo translate('comments'); ?></td>
							</tr>
						<?php
						foreach ($reported_links['internal'] as $assetid => $links) {
							foreach ($links as $link) {
								$from_info = $this->_getAssetReportInfo($assetid);
								$from_name = (strlen($from_info['name']) < 40) ? $from_info['name'] : ellipsisize($from_info['name'], 40);

								$to = NULL;
								if ($link['data']) {
									$to =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['data'], '', TRUE);
								}
								if (is_null($to)) {
									$to_type = '';
									$to_string = translate('unknown_asset_id', $link['data']);
									$to_bg = '';
									$to_status = '';
									$to_name = '';
								} else {
									$to_type = $to->type();
									$to_name = (strlen($to->name) < 40) ? $to->name : ellipsisize($to->name, 40);
									$to_string = '<span title="'.$to->name.'"> [#'.$to->id.'] '.$to_name.'</span>';
									$to_bg = get_asset_status_icon($to->status);
									$to_status = get_status_description($to->status);
								}

								?>
								<tr>
									<!-- ASSET STATUS -->
									<td class="sq-backend-table-cell" style="width: 120px; padding-top: 7px; padding-right: 0px;">
										<?php echo $from_info['bg']; ?> <?php echo $from_info['status']; ?>
									</td>

									<!-- ASSET ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($from_info['type'])) {
											?>
											<span title="<?php echo ucwords(str_replace('_', ' ', $from_info['type'])); ?>">
												<?php echo get_asset_type_icon($from_info['type']); ?>
											</span>
											<?php
										}
										?>
									</td>

									<!-- ASSET NAME -->
									<td class="sq-backend-table-cell" style="width: 260px; padding-right: 1px; padding-top:6px; text-align: left;">
										<a href="<?php echo $from_info['backend_url']; ?>" title="<?php echo translate('edit_this_asset'); ?>">[#<?php echo  $from_info['assetid']; ?>] <?php echo  $from_name ?></a>
									</td>

									<!-- LINKED STATUS -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-top: 7px; padding-right: 0px;">
											<?php echo $to_bg; ?><?php echo $to_status; ?>
									</td>

									<!-- LINKED ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($to_type)) {
											echo get_asset_type_icon($to_type);
										}
										?>
									</td>

									<!-- LINKED NAME -->
									<td class="sq-backend-table-cell" style="width: 190px; padding-right: 1px; padding-top:6px; text-align: left;">
										<?php echo $to_string; ?>
									</td>

									<!-- COMMENT -->
									<td class="sq-backend-table-cell">
										<?php
											if (!$link['valid']) {
												echo '<span style="color: #007EFA;"><b>';
											}
										echo $link['description'];
										if (!$link['valid']) echo '</b></span>';
										?>
									</td>
								</tr>
								<?php
							}//end foreach link
						}//end foreach asset links
						?>
						</table>
						<?php
					}//end else
				$o->closeField();
			$o->closeSection();
		}//end if internal section is true

		if ($display_options['external_section'] == TRUE) {
			$o->openSection(translate('external_links'));
				$o->openField('');
					if (empty($reported_links['external'])) {
						echo translate('cms_report_links_no_external_links');
					} else {
						$ast_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($reported_links['external']));
						?>
						<table class="sq-backend-table">
							<tr>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('found_in'); ?></td>
								<th class="sq-backend-table-header" style="width: 274px;"><?php echo translate('url'); ?></td>
								<th class="sq-backend-table-header"><?php echo translate('comments'); ?></td>
							</tr>
						<?php
						foreach ($reported_links['external'] as $assetid => $links) {
							foreach ($links as $link) {
								$from_info = $this->_getAssetReportInfo($assetid);
								$from_name = (strlen($from_info['name']) < 40) ? $from_info['name'] : ellipsisize($from_info['name'], 40);

								$name = (strlen($link['data']) < 50) ? $link['data'] : ellipsisize($link['data'], 50);
								$to_string = '<a href="'.$link['data'].'" target="_blank" title="'.$link['data'].'">'.$name.'</a>';
								?>
								<tr>
									<!-- ASSET STATUS -->
									<td class="sq-backend-table-cell" style="width: 120px; padding-top: 7px; padding-right: 0px;">
										<?php echo $from_info['bg']; ?> <?php echo $from_info['status']; ?>
									</td>

									<!-- ASSET ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($from_info['type'])) {
											?>
											<span title="<?php echo ucwords(str_replace('_', ' ', $from_info['type'])); ?>">
												<?php echo get_asset_type_icon($from_info['type']); ?>
											</span>
											<?php
										}
										?>
									</td>

									<!-- ASSET NAME -->
									<td class="sq-backend-table-cell" style="width: 190px; padding-right: 1px; padding-top:6px; text-align: left;">
										<a href="<?php echo $from_info['backend_url']; ?>" title="<?php echo translate('edit_this_asset') ?>">[#<?php echo (int) $from_info['assetid']; ?>] <?php echo $from_name; ?></a>
									</td>

									<!-- URL -->
									<td class="sq-backend-table-cell" style="width: 226px; padding-right: 1px; padding-top:6px; text-align:left;">
										<?php echo $to_string; ?>
									</td>

									<!-- COMMENTs -->
									<td class="sq-backend-table-cell">
										<?php
											if (!$link['valid']) {
												echo '<span style="color: #007EFA;"><b>';
											}
										echo $link['description'];
										if (!$link['valid']) echo '</b></span>';
										?>
									</td>
								</tr>
								<?php
							}//end foreach link
						}//end foreach asset links
						?>
						</table>
						<?php
					}//end else
				$o->closeField();
			$o->closeSection();
		}//end if external section is true

		if ($display_options['email_section'] == TRUE) {
			$o->openSection(translate('cms_report_links_email_links'));
				$o->openField('');
					if (empty($reported_links['mailto'])) {
						echo translate('cms_report_links_no_email_links');
					} else {
						?>
						<table class="sq-backend-table">
							<tr>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('found_in'); ?></td>
								<th class="sq-backend-table-header" style="width: 274px;"><?php echo translate('email'); ?></td>
								<th class="sq-backend-table-header"><?php echo translate('comments'); ?></td>
							</tr>
						<?php
						foreach ($reported_links['mailto'] as $assetid => $links) {
							foreach ($links as $link) {
								$from_info = $this->_getAssetReportInfo($assetid);
								$to_string = $link['data'];
								$name = (strlen($from_info['name']) < 40) ? $from_info['name'] : ellipsisize($from_info['name'], 40);
								?>
								<tr>

									<!-- ASSET STATUS -->
									<td class="sq-backend-table-cell" style="width: 120px; padding-top: 7px; padding-right: 0px;">
										<?php echo $from_info['bg']; ?> <?php echo $from_info['status']; ?>
									</td>

									<!-- ASSET ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($from_info['type'])) {
											?>
											<span title="<?php echo ucwords(str_replace('_', ' ', $from_info['type'])); ?>">
												<?php echo get_asset_type_icon($from_info['type']); ?>
											</span>
											<?php
										}
										?>
									</td>

									<!-- ASSET NAME -->
									<td class="sq-backend-table-cell" style="width: 190px; padding-right: 1px; padding-top:6px; text-align: left;">
										<a href="<?php echo $from_info['backend_url']; ?>" title="<?php echo translate('edit_this_asset'); ?>">[#<?php echo (int) $from_info['assetid']; ?>] <?php echo $name; ?></a>
									</td>

									<!-- ADDRESS -->
									<td class="sq-backend-table-cell" style="width: 226px; padding-right: 1px; padding-top:6px;">
										<?php echo $to_string; ?>
									</td>

									<!-- COMMENTS -->
									<td class="sq-backend-table-cell">
										<?php
											if (!$link['valid']) {
												echo '<span style="color: #007EFA;"><b>';
											}
										echo $link['description'];
										if (!$link['valid']) echo '</b></span>';
										?>
									</td>
								</tr>
								<?php
							}//end foreach link
						}//end foreach asset links
						?>
						</table>
						<?php
					}//end else
				$o->closeField();
			$o->closeSection();
		}//end if email_section is true

	}//end paintReport()


	/**
	* Get reporting information about an asset
	* Returns array('assetid', 'type', 'string', 'bg')
	*
	* @param string	$assetid	the id of the asset we are getting reporting info for
	*
	* @return array
	* @access private
	*/
	function _getAssetReportInfo($assetid)
	{
		$asset = NULL;
		if ($assetid) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
		}
		if (is_null($asset)) {
			$asset_type = '';
			$asset_name = 'Unknown asset #'.$assetid;
			$asset_string = 'Unknown asset #'.$assetid;
			$asset_bg = 'FFFFFF';
			$asset_status = '';
			$asset_url = '';
			$asset_preview_url = '';
		} else {
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id);
			$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset(array_pop($dependants));
			if (!is_null($parent)) {
				$asset_type = $parent->type();
				$asset_name = $parent->name;
				$asset_bg = get_asset_status_icon($parent->status);
				$asset_status = get_status_description($parent->status);
				$assetid = $parent->id;
				$asset_url = $parent->getBackendHref();
				$asset_preview_url = $parent->getURL();
			} else {
				$asset_type = $asset->type();
				$asset_name = $asset->name;
				$asset_bg = get_asset_status_icon($asset->status);
				$asset_status = get_status_description($asset->status);
				$asset_url = $asset->getBackendHref();
				$asset_preview_url = $asset->getURL();
			}

			include_once SQ_FUDGE_PATH.'/general/general.inc';
			$asset_string = ellipsisize($asset_name, 40);

			$asset_string = '
				<a href="'.$asset_preview_url.'" target="_blank" title="'.$asset_name.'">'.$asset_string.'</a>';
		}

		return Array(
				'assetid'		=> $assetid,
				'name'			=> $asset_name,
				'type'			=> $asset_type,
				'string'		=> $asset_string,
				'bg'			=> $asset_bg,
				'status'		=> $asset_status,
				'backend_url'	=> $asset_url,
			   );

	}//end _getAssetReportInfo()


	/**
	* Paints the display options available for this report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintDisplayOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$tick = sq_web_path('lib').'/web/images/tick.gif';
		$cross = sq_web_path('lib').'/web/images/cross.gif';
		$current_settings = $asset->attr('display_options');

		?>
		<script type="text/javascript">
			var prefix = '<?php echo $prefix; ?>';

			// Disables a sections child options, if the top option is deselected
			function change_section(section, obj) {
				var start = prefix + '_' + section + '_';
				document.getElementById(start + 'working').disabled = obj.checked ? false : true;
				document.getElementById(start + 'broken').disabled = obj.checked ? false : true;
			}
		</script>

		<table class="sq-backend-table" style="width: 300px;">
			<tr>
				<th class="sq-backend-table-header" colspan="2"><?php echo translate('cms_report_links_internal_links'); ?></td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_section', 'Internal'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['internal_section'];
						if ($write_access) {
							check_box($prefix.'_internal_section', '1', $value, 'change_section("internal", this)');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_broken'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['internal_broken'];
						if ($write_access) {
							check_box($prefix.'_internal_broken', '1', $value, '', $current_settings['internal_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_working'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['internal_working'];
						if ($write_access) {
							check_box($prefix.'_internal_working', '1', $value, '', $current_settings['internal_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<th class="sq-backend-table-header" colspan="2"><?php echo translate('cms_report_links_external_links'); ?></td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_section', 'External'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['external_section'];
						if ($write_access) {
							check_box($prefix.'_external_section', '1', $value, 'change_section("external", this)');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_broken'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['external_broken'];
						if ($write_access) {
							check_box($prefix.'_external_broken', '1', $value, '', $current_settings['external_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_working'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['external_working'];
						if ($write_access) {
							check_box($prefix.'_external_working', '1', $value, '', $current_settings['external_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<th class="sq-backend-table-header" colspan="2"><?php echo translate('cms_report_links_email_links'); ?></td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_section', 'Email'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['email_section'];
						if ($write_access) {
							check_box($prefix.'_email_section', '1', $value, 'change_section("email", this)');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_broken'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['email_broken'];
						if ($write_access) {
							check_box($prefix.'_email_broken', '1', $value, '', $current_settings['email_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_working'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['email_working'];
						if ($write_access) {
							check_box($prefix.'_email_working', '1', $value, '', $current_settings['email_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>

		</table>

		<?php
		return $write_access;

	}//end paintDisplayOptions()


	/**
	* Paints the display options available for this report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDisplayOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return $write_access;
		$display_options = $asset->attr('display_options');

		// These conditions below ensure that the elements retain their values if they are disabled
		$display_options['internal_section'] = isset($_POST[$prefix.'_internal_section']);
		if ($display_options['internal_section']) {
			$display_options['internal_broken'] = isset($_POST[$prefix.'_internal_broken']);
			$display_options['internal_working'] = isset($_POST[$prefix.'_internal_working']);
		}

		$display_options['external_section'] = isset($_POST[$prefix.'_external_section']);
		if ($display_options['external_section']) {
			$display_options['external_broken'] = isset($_POST[$prefix.'_external_broken']);
			$display_options['external_working'] = isset($_POST[$prefix.'_external_working']);
		}

		$display_options['email_section'] = isset($_POST[$prefix.'_email_section']);
		if ($display_options['email_section']) {
			$display_options['email_broken'] = isset($_POST[$prefix.'_email_broken']);
			$display_options['email_working'] = isset($_POST[$prefix.'_email_working']);
		}

		$asset->setAttrValue('display_options', $display_options);
		return TRUE;

	}//end processDisplayOptions()


	/**
	* Paints the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExport(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		echo hidden_field($prefix.'_export_switch', '0');

		echo normal_button($prefix.'_export', translate('download_file'), 'this.form.'.$prefix.'_export_switch.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_export_switch.value = 0; return true;');

		return TRUE;

	}//end paintExport()


	/**
	* Processes the export block
	*
	* This will generate a CSV file, and present a download box to the user
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExport(&$asset, &$o, $prefix)
	{
		if (!$_POST[$prefix.'_export_switch']) return TRUE;

		require_once SQ_FUDGE_PATH.'/csv/csv.inc';
		$csv =& new CSV();
		$csv->setFilename($asset->name.'_report.csv');
		$csv->setDeliminator(',');

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$logs = Array();

		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$report_path = $asset->data_path.'/report.xml';

		if (!is_file($report_path)) {
			echo 'This report has not been generated';
			return;
		}

		$input =& new XML_Tree($report_path);
		$root =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_localised_error('CORE0092', E_USER_WARNING, $report_path, $root->getMessage(), $root->getUserInfo());
			return;
		}

		$headers = Array();
		$empty_headers = TRUE;
		$top_headers = TRUE;

		foreach ($root->children as $asset_node) {
			$persistent_values = Array();
			// Have to initialise the headers
			foreach ($asset_node->attributes as $attr => $value) {
				if ($top_headers) $headers[] = $attr;
				// Need to setup the persistent values for each of the nodes (like source asset, type etc)
				$persistent_values[] = $value;
			}
			$top_headers = FALSE;

			foreach ($asset_node->children as $asset_data) {
				$row = $persistent_values;
				foreach ($asset_data->attributes as $attr => $value) {
					if ($empty_headers) $headers[] = $attr;
					$row[] = $value;
				}
				foreach ($asset_data->children as $data) {
					if ($empty_headers) $headers[] = $data->name;
					$row[] = $data->content;
				}
				$logs[] = $row;
				$empty_headers = FALSE;
			}
		}

		foreach ($headers as $id => $value) {
			$headers[$id] = ucwords(str_replace('_', ' ', $value));
		}

		$csv->setFieldHeaders($headers);

		$csv->setValues($logs);

		// export with keyed fields
		$csv->export(TRUE);
		exit(0);

		return TRUE;

	}//end processExport()


}//end class

?>
