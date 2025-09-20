from config import Config

def main():
    cfg = Config()
    print(f"Starting application on port {cfg.port} with version {cfg.version}")

    

if __name__ == "__main__":
    main()