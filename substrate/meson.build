headers = [
	'fd',
	'memfd',
	'units',
	'utility',
	'enum_utils',
	'fixed_vector',
	'bounded_iterator',
	'string',
	'managed_ptr',
	'socket',
	'console',
	'pty',
	'pipe',
	'mmap',
	'span'

]

advanced_headers = [
	'advanced/io',
	'advanced/fd',
	'advanced/guard_page'
]

internal_headers = [
	'internal/defs',
	'internal/cxx11_compat',
	'internal/fd_compat'
]

install_headers(
	headers,
	subdir: 'substrate'
)

install_headers(
	advanced_headers,
	subdir: 'substrate/advanced'
)

install_headers(
	internal_headers,
	subdir: 'substrate/internal'
)
