"""Black-box regression tests for member A. Python standard library only."""
import argparse
import os
from pathlib import Path
import subprocess
import unittest

parser = argparse.ArgumentParser()
parser.add_argument('--build-dir', default='build')
options, remaining = parser.parse_known_args()
BUILD = Path(options.build_dir).resolve()
SUFFIX = '.exe' if os.name == 'nt' else ''


class ApplicationTests(unittest.TestCase):
    def run_program(self, stdin='', *, binary='pizza-demo', args=(), env=None, code=0):
        environment = {k: v for k, v in os.environ.items() if not k.startswith('PIZZA_TEST_')}
        environment.update(env or {})
        result = subprocess.run(
            [str(BUILD / (binary + SUFFIX)), *args],
            input=stdin.encode() if isinstance(stdin, str) else stdin,
            capture_output=True, env=environment, timeout=10,
        )
        output = result.stdout.decode(errors='replace') + result.stderr.decode(errors='replace')
        self.assertEqual(result.returncode, code, output)
        return output

    def test_real_build_has_no_demo_login(self):
        out = self.run_program('1\n1001\ncustomer-demo\n0\n', binary='pizza-ordering-system')
        self.assertIn('not available yet', out)
        self.assertNotIn('Login successful', out)
        self.assertNotIn('[DEMO]', out)

    def test_help_version_and_invalid_argument(self):
        self.assertIn('Usage:', self.run_program(binary='pizza-ordering-system', args=['--help']))
        self.assertIn('0.2.0', self.run_program(binary='pizza-ordering-system', args=['--version']))
        self.assertIn('Unknown argument', self.run_program(binary='pizza-ordering-system', args=['--demo'], code=2))

    def test_customer_routes_and_identity(self):
        out = self.run_program('1\n1001\ncustomer-demo\n1\n2\n3\n4\n5\n6\n0\n0\n')
        self.assertIn('Login successful', out)
        self.assertIn('[DEMO] menu_show customer=0', out)
        for operation in ('order_place', 'order_history', 'wallet_deposit', 'account_view_profile', 'account_edit_profile'):
            self.assertIn(f'[DEMO] {operation} customer=1001', out)
        self.assertNotIn('=== Shop manager |', out)
        self.assertIn('Logged out.', out)

    def test_manager_routes(self):
        out = self.run_program('3\n9001\nmanager-demo\n1\n2\n3\n4\n5\n6\n0\n0\n')
        for operation in ('menu_manage', 'pricing_manage', 'account_add_customer', 'account_remove_customer', 'account_edit_customer', 'stats_show'):
            self.assertIn(f'[DEMO] {operation} customer=0', out)
        self.assertNotIn('=== Customer |', out)

    def test_failed_and_wrong_role_login(self):
        for sequence in ('1\n1001\nwrong\n0\n', '3\n1001\ncustomer-demo\n0\n', '1\n9001\nmanager-demo\n0\n'):
            with self.subTest(sequence=sequence):
                out = self.run_program(sequence)
                self.assertIn('Incorrect account', out)
                self.assertNotIn('Login successful', out)

    def test_malformed_auth_provider(self):
        for mode in ('wrong_role', 'wrong_id', 'zero_id'):
            with self.subTest(mode=mode):
                out = self.run_program('1\n1001\ncustomer-demo\n0\n', env={'PIZZA_TEST_AUTH': mode})
                self.assertIn('invalid authentication result', out)
                self.assertNotIn('Login successful', out)

    def test_logout_then_failed_login_clears_old_identity(self):
        out = self.run_program('1\n1001\ncustomer-demo\n0\n3\n9001\nwrong\n0\n')
        self.assertEqual(out.count('Login successful'), 1)
        self.assertEqual(out.count('=== Customer |'), 1)
        self.assertNotIn('=== Shop manager |', out)

    def test_registration_and_cancel_login(self):
        out = self.run_program('2\n1\n0\n3\n9001\n\n0\n')
        self.assertIn('[DEMO] account_register', out)
        self.assertNotIn('Login successful', out)

    def test_invalid_and_overlong_input_recovery(self):
        sequence = '\nabc\n-1\n4\n1x\n1.2\n' + '9' * 500 + '\n0\n'
        out = self.run_program(sequence)
        self.assertIn('Enter a whole number', out)
        self.assertIn('too long', out)
        self.assertIn('Goodbye', out)
        out = self.run_program(b'1\x00\n0\n')
        self.assertIn('control characters', out)
        self.assertNotIn('Account number (0 to cancel)', out)

    def test_eof_at_every_screen(self):
        for sequence in ('', '1\n', '1\n1001\n', '1\n1001\ncustomer-demo\n', '3\n9001\nmanager-demo\n'):
            with self.subTest(sequence=sequence):
                out = self.run_program(sequence)
                self.assertEqual(out.count('[DEMO] storage_save'), 1)
                self.assertIn('Goodbye', out)

    def test_load_failure_does_not_save_or_open_menu(self):
        for mode in ('io_error', 'data_error'):
            with self.subTest(mode=mode):
                out = self.run_program(env={'PIZZA_TEST_LOAD': mode}, code=1)
                self.assertIn('Cannot open shop data', out)
                self.assertNotIn('[DEMO] storage_save', out)
                self.assertNotIn('Customer login', out)

    def test_save_failure_retry(self):
        out = self.run_program('0\n1\n', env={'PIZZA_TEST_SAVE': 'once'})
        self.assertEqual(out.count('[DEMO] storage_save'), 2)
        self.assertIn('NOT been confirmed saved', out)
        self.assertNotIn('Exiting with unsaved data', out)

    def test_save_failure_exit_and_eof(self):
        for sequence in ('0\n0\n', '0\n'):
            out = self.run_program(sequence, env={'PIZZA_TEST_SAVE': 'always'}, code=1)
            self.assertIn('Exiting with unsaved data', out)

    def test_module_error_and_cancellation_return_to_menu(self):
        for mode, message in [('io_error', 'A read or write failed'), ('cancelled', 'Operation cancelled')]:
            out = self.run_program('1\n1001\ncustomer-demo\n2\n0\n0\n', env={'PIZZA_TEST_ACTION': mode})
            self.assertIn(message, out)
            self.assertEqual(out.count('=== Customer |'), 2)

    def test_many_login_logout_cycles(self):
        out = self.run_program(('1\n1001\ncustomer-demo\n0\n' * 150) + '0\n')
        self.assertEqual(out.count('Logged out.'), 150)

    def test_text_boundaries_crlf_and_eof(self):
        for sequence in ('abcd\n', 'abcd\r\n', 'abcd'):
            self.assertIn('RESULT 1 abcd', self.run_program(sequence, binary='test-core', args=['text']))
        for sequence in ('abcde\nabcd\n', '\n  \nabcd\n', 'a\rb\nabcd\n'):
            self.assertIn('RESULT 1 abcd', self.run_program(sequence, binary='test-core', args=['text']))
        self.assertIn('RESULT 0', self.run_program('', binary='test-core', args=['text']))

    def test_money_and_confirmation_helpers(self):
        out = self.run_program('-1\n0\n1.001\n1000.01\n12.30\n', binary='test-core', args=['money'])
        self.assertIn('RESULT 1 1230', out)
        self.assertIn('RESULT 1 1', self.run_program('maybe\nY\n', binary='test-core', args=['confirm']))
        self.assertIn('RESULT 1 0', self.run_program(' n \n', binary='test-core', args=['confirm']))
        self.assertIn('RESULT 0', self.run_program('', binary='test-core', args=['money']))

    def test_integer_final_line_without_newline(self):
        self.assertIn('RESULT 1 6', self.run_program('6', binary='test-core', args=['int']))
        self.assertIn('RESULT 1 2', self.run_program('9999999999999999999999\n2\n', binary='test-core', args=['int']))


if __name__ == '__main__':
    unittest.main(argv=['test_app.py', *remaining], verbosity=2)
