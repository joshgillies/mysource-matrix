<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: condition_user_ipv6_edit_fns.inc,v 1.1 2012/10/11 02:36:11 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/csv/csv.inc';
require_once dirname(__FILE__).'/condition_user_ipv6.inc';


/**
* Condition_User_IPv6_Edit_Fns
*
* Edit functions for the user IPv6 condition
*
* @see Asset_Edit_Fns
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_IPv6_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the backend editing interface for this condition
	*
	* @param array				&$condition_data	Array of settings for this condition
	* @param Backend_Outputter	$o					Backend outputter to use
	* @param string				$prefix				Prefix for form elements
	* @param boolean			$write_access		Whether we have access to edit the field values
	*
	* @return void
	* @access public
	*/
	public static function paintEditInterface(Array $condition_data, Backend_Outputter $o, $prefix, $write_access)
	{
		if (!isset($condition_data['user_ip_ranges'])) {
			$condition_data['user_ip_ranges'] = Array();
		}

		if (!isset($condition_data['default_grant'])) {
			$condition_data['default_grant'] = FALSE;
		}

		if ($write_access) {
			$condition_data['user_ip_ranges']['new'] = Array('ip_address' => NULL, 'cidr' => '64', 'grant' => 1);
		}

		$grant_options = Array(1 => translate('Grant'), 0 => translate('Deny'));



		if (!$write_access && empty($condition_data['user_ip_ranges'])) {
			?><em><?php echo translate('There are no conditions set'); ?></em><?php

		} else {
			?>
			<table class="sq-backend-table">
				<tr>
				<?php if ($write_access) {
				?>
					<th></th>
				<?php
				}
				?>
				<th><?php echo translate('Network IP Address (IPv6)'); ?></th>

				<th></th>
				<th><?php echo translate('CIDR'); ?></th>

				<th><?php echo translate('Action'); ?></th>

				<?php if ($write_access) {
				?>
					<th><?php echo translate('Delete?'); ?></th>

				<?php
				}
				?>
				</tr>
				<?php foreach ($condition_data['user_ip_ranges'] as $index => $ranges) {
				?>
					<tr>
						<?php if ($write_access) {
						?>
						<td>
							<?php
							if ($index === 'new') {
								echo '<strong>'.translate('New?').'</strong>';

							}
							?>
						</td>
						<?php
						}
						?>
						<td>
							<?php
							if ($write_access) {
								text_box($prefix.'['.$index.'][ip_address]', $ranges['ip_address'], 42, 40);
							} else {
								echo $ranges['ip_address'];
							}
							?>
						</td>
						<td>
						/
						</td>
						<td>
							<?php
							if ($write_access) {
								text_box($prefix.'['.$index.'][cidr]', $ranges['cidr'], 5, 3);
							} else {
								echo $ranges['cidr'];
							}
							?>
						</td>
						<td>
						<?php
							if ($write_access) {
								?><?php
									combo_box($prefix.'['.$index.'][grant]', $grant_options, FALSE, Array($ranges['grant']));
								?><?php
							} else {
								?><strong style="color: <?php echo $ranges['grant'] ? 'green' : 'red' ?>"><?php
								echo $grant_options[$ranges['grant']];
								?></strong><?php
							}
						?></td>
						<td style="text-align: center">
						<?php
							if ($write_access && ($index !== 'new')) {
								check_box($prefix.'['.$index.'][delete]', FALSE);
							} else {
								echo '&nbsp;';
							}
						?></td>
					</tr>
				<?php
				}//end foreach
				?>
			</table>
		<?php
		}//end else

		echo '<p>';
		echo translate('If IP address is outside one of these ranges').': ';

		if ($write_access) {
			combo_box($prefix.'[default_grant]', $grant_options, FALSE, Array($condition_data['default_grant']));
		} else {
			?><strong style="color: <?php echo $condition_data['default_grant'] ? 'green' : 'red' ?>"><?php
			echo $grant_options[$condition_data['default_grant']];
			?></strong><?php

		}
		echo '</p>';

		echo '<p>';
		echo '<b>'.translate('Import rules from CSV file').':</b>&nbsp;';

		file_upload($prefix.'_import');
		echo '</p>';
		$o->note(translate('The file uploaded should be a CSV file without a header row. Each line should contain exactly 3 fields: the first being the network IP address, the second being the subnet mask in the CIDR notation, and the third being a one (1) for a Grant rule or zero (0) for a Deny rule. The default Grant/Deny setting is not affected by any upload and should be set separately.'));

		$o->note(translate('The network address entered will be automatically lowercased and converted to the "compressed" format. For example, all of these IPv6 addresses "00ff:0:0:0:0:0:0:0", "FF:0::0", "00ff::0000" will be converted to "ff::".'));


	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param Backend_Outputter	$o		Backend outputter to use
	* @param string				$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	public static function processEditInterface(Backend_Outputter $o, $prefix)
	{
		if (isset($_FILES[$prefix.'_import']) && ($_FILES[$prefix.'_import']['error'] == 0)) {
			$file_name = $_FILES[$prefix.'_import']['tmp_name'];
			if (is_uploaded_file($file_name)) {
				$condition_data = self::importRulesFromCSV($file_name);
				if ($condition_data !== FALSE) {
					$condition_data['default_grant'] = $_POST[$prefix]['default_grant'];
					return $condition_data;
				} else {
					trigger_localised_error('CORE0249', E_USER_WARNING);
				}
			}
		}

		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$ip_ranges = Array();

		foreach ($_POST[$prefix] as $index => $post_data) {
			if (!isset($post_data['delete'])&& is_array($post_data)) {
				$this_row = Array(
								'ip_address'	=> self::processIPAddressField($post_data['ip_address']),
								'cidr'			=> self::processIPRangeCIDRField($post_data['cidr']),
								'grant'			=> $post_data['grant'],
							);

				// IP address not filled in => continue with life
				if (empty($this_row['ip_address'])) {
					if (!empty($post_data['ip_address'])) {
						trigger_error('Specified network address "'.$post_data['ip_address'].'" is not valid', E_USER_WARNING);
					}
					continue;
				}

				if (empty($this_row['cidr']) && $this_row['cidr'] !== '0') {
					trigger_error('Specified CIDR "'.$post_data['cidr'].'" is not valid', E_USER_WARNING);
					continue;
				}


				// see if there is an exact duplicate, skip it if there is
				if (in_array($this_row, $ip_ranges)) {
					continue;
				}

				// now flip the grant field to see if there is a IP/subnet dupe
				// of the other kind of rule
				$this_row['grant'] = !$this_row['grant'];
				if (in_array($this_row, $ip_ranges)) {
					trigger_localised_error('CORE0250', E_USER_WARNING, $this_row['ip_address'], $this_row['cidr']);
					continue;
				}

				// Check to see whether the ip address is a valid network address
				// for the subnet mask in question
				if (self::_doesIPBeginSubnet($this_row['ip_address'], $this_row['cidr'])) {
					// put grant back to where it was before and store it
					$this_row['grant'] = $post_data['grant'];
					$ip_ranges[] = $this_row;
				} else {
					trigger_localised_error('CORE0248', E_USER_WARNING, $this_row['ip_address'], $this_row['cidr']);
				}
			}//end if
		}//end foreach

		$results = Array(
					'user_ip_ranges'	=> $ip_ranges,
					'default_grant'		=> $_POST[$prefix]['default_grant'],
				   );

		return $results;

	}//end processEditInterface()


	/**
	* Processes an IP address field
	*
	* @param array	$octets	the octets from the form response
	*
	* @return mixed int|boolean
	* @access public?
	*/
	public static function processIPAddressField($ip_address)
	{
		$ip_address = strtolower(trim($ip_address));
		if (!empty($ip_address)) {
			// Store the IP in the compressed format notation
			$ip_address = compressIPv6Address($ip_address);
		}

		return $ip_address;

	}//end processIPAddressField()


	/**
	* Processes a CIDR notation value representing IPv6 network subnet mask
	*
	* @param array	$octets	the data from the form request
	*
	* @return mixed string|NULL
	* @access public?
	*/
	public static function processIPRangeCIDRField($cidr)
	{
		$cidr = trim($cidr);

		if ($cidr <= 128 && $cidr >= 0) {
			return $cidr;
		}

		return NULL;

	}//end processSubnetMaskField()


	/**
	* Returns TRUE if the IP address is a part of the subnet specified
	*
	* @param string	$ip_address	the IP address of the remote user
	* @param string	$network_ip	the network IP address to test against
	* @param string	$cidr		the subnet mask in CIDR notation
	*
	* @return boolean
	* @access public
	*/
	public static function isInSubnet($ip_address, $network_ip, $cidr)
	{
		$remote_ip_bits = ipv6ToBits($ip_address);
		$network_ip_bits = ipv6ToBits($network_ip);

		if (substr($remote_ip_bits, 0, $cidr) === substr($network_ip_bits, 0, $cidr)) {
			return TRUE;
		}

		return FALSE;

	}//end isInSubnet()


	/**
	* Sort IP range list in order from largest subnet to smallest subnet
	*
	* @param array	$ip_ranges	unsorted list of IP addresses
	*
	* @return array
	* @access public
	*/
	public static function sortByCIDROrder($ip_ranges)
	{
		$ip_ranges_by_cidr = Array();

		foreach ($ip_ranges as $ip_range) {
			$ip_ranges_by_cidr[$ip_range['cidr']][] = $ip_range;
		}

		ksort($ip_ranges_by_cidr);

		$ip_ranges = Array();
		foreach ($ip_ranges_by_cidr as $cidr_ip_range) {
			$ip_ranges = array_merge($ip_ranges, $cidr_ip_range);
		}

		return $ip_ranges;

	}//end sortByCIDROrder()


	/**
	* Returns TRUE if the IP address given begins in the passed CIDR
	* representing the subnet mask
	*
	* @param string	$ip_address	The IP address
	* @param string	$cidr		Subnet mask in the CIDR notation
	*
	* @return boolean
	* @access private
	*/
	protected static function _doesIPBeginSubnet($ip_address, $cidr)
	{
		$ip_bits = ipv6ToBits($ip_address);

		// If the IP begins in the subnet, then all the remaining (128-cidr) IP bits should be zero
		if ((int) substr($ip_bits, $cidr) === 0) {
			return TRUE;
		}

		return FALSE;

	}//end _doesIPBeginSubnet()


	/**
	* import rules from a CSV file - returns array of condition data
	* or FALSE if CSV file is invalid
	*
	* @param string	$file_name	the (temporary) file name to import the CSV from
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function importRulesFromCSV($file_name)
	{
		// CSV format: first field = IP address (IPv6), second = subnet CIDR, third = grant/deny
		$condition_data = Array('user_ip_ranges' => Array());
		$condition_data['default_grant'] = 0;

		$csv = new CSV();
		$csv->setFilepath($file_name);
		$csv->import();
		$lines = $csv->values;

		// where we will be building our IP ranges
		$ip_ranges = Array();

		foreach ($lines as $line_no => $line_fields) {
			$line_no += 1;

			// doesn't have correct number of fields...
			if (count($line_fields) != 3) {
				trigger_error('CSV file does not has expected number of fields', E_USER_WARNING);
				return FALSE;
			}

			$ip_address = self::processIPAddressField($line_fields[0]);
			if (empty($ip_address )) {
				trigger_error('Network address specifed for the network range in CSV file at line '.$line_no.' is not valid', E_USER_WARNING);
				return FALSE;
			}
			$cidr = self::processIPRangeCIDRField($line_fields[1]);
			if (empty($cidr )) {
				trigger_error('CIDR specifed for the network range in CSV file at line '.$line_no.' is not valid', E_USER_WARNING);
				return FALSE;
			}

			$grant = $line_fields[2];

			$ip_range_field = Array(
								'ip_address'	=> $ip_address,
								'cidr'		=> $cidr,
								'grant'			=> $grant,
							  );

			// this is already in the file - ignore it
			if (in_array($ip_range_field, $ip_ranges)) continue;

			// this has already been seen with a different grant!!
			// complain, then ignore it
			$ip_range_field['grant'] = !$ip_range_field['grant'];
			if (in_array($ip_range_field, $ip_ranges)) {
				trigger_localised_error('CORE0250', E_USER_WARNING, $ip_range_field['ip_address'], $ip_range_field['cidr']);
				continue;
			}

			// is it a valid combination?
			if (!self::_doesIPBeginSubnet($ip_range_field['ip_address'], $ip_range_field['cidr'])) {
				trigger_localised_error('CORE0248', E_USER_WARNING, $ip_range_field['ip_address'], $ip_range_field['cidr']);
				continue;
			}

			// flip the grant back to what it was, and add it to the list
			$ip_range_field['grant'] = !$ip_range_field['grant'];
			$ip_ranges[] = $ip_range_field;
		}//end foreach

		$condition_data['user_ip_ranges'] = $ip_ranges;

		return $condition_data;

	}//end importRulesFromCSV()

}//end class

?>
