# Key-value-store service

[Return to document](./README.md)

## Start service

```sh
cd Server
make
./build/app
```

## Test

```sh
cd Client
make
```

- For single test command line
    ```sh
    ./Client -S
    ```

  - Options:
    - `set <key> <value>`
    - `get <key>`
    - `del <key>`
- For single client multiple request
    ```sh
    ./Client -S <-s|-g|-d>
    ```

- For multiple client test
    ```sh
    ./Client -m <-s|-g|-d>
    ```

- For measuring time of command with N keys
    ```sh
    ./Client -B <-s|-g|-d> [N]
    ```